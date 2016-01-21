#include <omp.h>
#include <algorithm>

#include "graph.hpp"
#include "analysis.hpp"

using type_threadcount = analysis::type_time;

std::string Graph::getName(){
    return "locallist";
}

// PRE:		
// POST:	locallist is appended to globallist, locallist is empty
inline void gatherlist(Graph::type_nodelist& globallist, Graph::type_nodelist& locallist, analysis::type_threadcount id) {
	#if DEBUG >= 2
		#pragma omp single
		std::cout << "\nBEFOREGATHER -\tglobal:" << globallist;
		#pragma omp barrier
		#pragma omp critical
		std::cout << "\nBEFOREGATHER -\tthread " << id << " :" << locallist;
		#pragma omp barrier
	#endif // DEBUG >= 2
	
	#pragma omp critical
	{
		globallist.splice(globallist.end(),locallist);
	}
	assert(locallist.empty());

	#if DEBUG >= 2
		#pragma omp single
		std::cout << "\nAFTERGATHER -\tglobal:" << globallist;
	#endif // DEBUG >= 2
}

// PRE:		globallist has all nodes that need to be distributed, locallist is empty
// POST:	locallist holds a maximum of n elements that were previously in globallist
inline void scatterlist( Graph::type_nodelist& globallist, Graph::type_nodelist& locallist, Graph::type_size n, analysis::type_threadcount id) {

	#if DEBUG >= 1
		#pragma omp single
		std::cout << "\nBEFORESCATTER -\tglobal:" << globallist;
	#endif // DEBUG >= 1

	Graph::type_nodelist::iterator start, end;
	#pragma omp critical
	{
		assert(locallist.empty());
		Graph::type_size len = globallist.size();
		n = std::min(n,len); // make sure that we do not take more nodes than present
		start = globallist.begin();
		end = globallist.begin();
		std::advance(end,n);
		locallist.splice(locallist.end(),globallist,start,end);
	}
	
	#if DEBUG >= 2
		#pragma omp single
		std::cout << "\nAFTERSCATTER -\tglobal:" << globallist;
	#endif // DEBUG >= 2
	#if DEBUG >= 1
		#pragma omp barrier
		#pragma omp critical
		std::cout << "\nAFTERSCATTER -\tthread " << id << " :" << locallist;
		#pragma omp barrier
	#endif // DEBUG >= 1

}

// PRE: 
// POST:	returns unsigned(ceil(n/d)), i.e. round-up division
inline unsigned roundupdiv(unsigned n, unsigned d) {
	return (1 + (n-1)/d);
}


void Graph::topSort() {

	// Sorting Magic happens here

	// SHARED VARIABLES
	type_size syncVal = 0;
	type_size nCurrentNodes = 0;
	type_nodelist currentnodes;

	// Start: currentnodes = root nodes 
	for(type_size i=0; i<N_; ++i) {
		if(nodes_[i]->getValue()==1) currentnodes.push_back(nodes_[i]);
	}
	nCurrentNodes = currentnodes.size();
	
	// Spawn OMP threads
	#pragma omp parallel shared(syncVal, nCurrentNodes, currentnodes)
	{
		
		// THREAD PRIVATE VARIABLES
		const int nThreads = omp_get_num_threads();
		const int threadID = omp_get_thread_num();
		type_nodelist currentnodes_local;
		type_nodelist solution_local;
		
		type_nodeptr parent;
		type_nodeptr child;
		type_size childcount = 0;
		type_size currentvalue = 0;

		A_.initialnodes(threadID,currentnodes_local.size());
		
		A_.starttiming(analysis::BARRIER);
		#pragma omp barrier // make sure everything is set up alright
		A_.stoptiming(threadID,analysis::BARRIER);
		
		while(!currentnodes.empty()) {

			#pragma omp single
			++syncVal;

			#if VERBOSE>=2
			#pragma omp single
			std::cout << "\nCurrent syncVal = " << syncVal;
			#endif // VERBOSE>=2
			
			#pragma omp single
			{
				nCurrentNodes = currentnodes.size();
                A_.frontSizeHistogram(nCurrentNodes);
			}
			A_.starttiming(analysis::BARRIER);
			#pragma omp barrier // make sure that nCurrentNodes is set
			A_.stoptiming(threadID,analysis::BARRIER);
		
			A_.starttiming(analysis::CURRENTSCATTER);
			scatterlist(currentnodes,currentnodes_local,roundupdiv(nCurrentNodes,nThreads), threadID);
			A_.stoptiming(threadID,analysis::CURRENTSCATTER);

			while(!currentnodes_local.empty()) {
				
				A_.incrementProcessedNodes(threadID);
				
				parent = currentnodes_local.front();
				currentvalue = parent->getValue();

				if(currentvalue>syncVal) {
					assert(currentvalue == syncVal+1);
					break;
				} else {
					solution_local.push_back(parent); // put node in solution
					currentnodes_local.pop_front(); // remove current node - already visited
				}

				++currentvalue; // increase value for child nodes
				childcount = parent->getChildCount();
                A_.incrementProcessedEdges(threadID, childcount);
				bool flag;
				for(type_size c=0; c<childcount; ++c) {
					child = parent->getChild(c);

					// Checking if last parent trying to update
					A_.starttiming(analysis::REQUESTVALUEUPDATE);
					flag = child->requestValueUpdate(); // IMPORTANT: control atomicity using OPTIMISTIC flag
					A_.stoptiming(threadID,analysis::REQUESTVALUEUPDATE);
					
					if(flag) { // last parent checking child
						currentnodes_local.push_back(child); // add child node at end of queue
						child->setValue(currentvalue); // set value of child node to parentvalue
					} 
			
				}
			}

			// Collect local lists in global list
			A_.starttiming(analysis::CURRENTGATHER);
			gatherlist(currentnodes,currentnodes_local,threadID);
			A_.stoptiming(threadID,analysis::CURRENTGATHER);
			A_.starttiming(analysis::SOLUTIONPUSHBACK);
			gatherlist(solution_,solution_local,threadID);
			A_.stoptiming(threadID,analysis::SOLUTIONPUSHBACK);
			
			A_.starttiming(analysis::BARRIER);
			#pragma omp barrier
			A_.stoptiming(threadID,analysis::BARRIER);
			
		}
	
	} // end of OMP parallel

	depth_ = syncVal;

}

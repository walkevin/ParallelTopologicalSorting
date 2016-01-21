#include <omp.h>

#include "graph.hpp"
#include "analysis.hpp"

std::string Graph::getName(){
    return "bitset";
}

void Graph::topSort() {
	// Sorting Magic happens here
	
    int nThreads;
    #pragma omp parallel
    {
        nThreads = omp_get_num_threads();
    } 
    // Indicator vector true if node is a current node (aka frontier node)
    std::vector<char> isCurrentNode(2*N_, false); //std::vector<bool> is not thread-safe
    std::vector<char> newChildrenPerThread(nThreads, true);
    bool newChildren = true;
    int shift = 0;
	// Spawn OMP threads
	#pragma omp parallel
	{
		// Declare Thread Private Variables
		const int threadID = omp_get_thread_num();
		type_nodelist solution_local;

		// Distribute Root Nodes among Threads
        #pragma omp for
		for(unsigned i=0; i<N_; ++i) {
			if(nodes_[i]->getValue()==1)
                isCurrentNode[i] = true;
		} // implicit barrier
    
        while(newChildren){
            newChildrenPerThread[threadID] = false;
            #if ENABLE_ANALYSIS == 1
                int shiftN = shift * N_;
                #pragma omp single
                A_.frontSizeHistogram(std::count(isCurrentNode.begin() + shiftN, isCurrentNode.begin() + shiftN + N_, true));
            #endif
            #pragma omp for schedule(dynamic, 256)
            for(size_t i = 0; i < N_; ++i){
                int idx = shift * N_ + i;
                if(!isCurrentNode[idx])
                    continue;
                
                A_.incrementProcessedNodes(threadID);

                auto parent = nodes_[i];

                solution_local.push_back(parent);
                isCurrentNode[idx] = false;// remove current node - already visited

				auto childcount = parent->getChildCount();
                A_.incrementProcessedEdges(threadID, childcount);
				for(unsigned c=0; c<childcount; ++c) {
					auto child = parent->getChild(c);

					// Checking if last parent trying to update
                    A_.starttiming(analysis::REQUESTVALUEUPDATE);
                    auto flag = child->requestValueUpdate(); // This call is thread-safe
                    A_.stoptiming(threadID, analysis::REQUESTVALUEUPDATE);
                    if(flag) { // last parent checking child
                        newChildrenPerThread[threadID] = true;
                        isCurrentNode[((shift+1)%2) * N_ + child->getID()] = true;// mark child as queued
					} 
				}
			}// end for => one frontier completed       
            A_.starttiming(analysis::SOLUTIONPUSHBACK);
            #pragma omp critical
            {
                solution_.splice(solution_.end(),solution_local);
            }
            A_.stoptiming(threadID, analysis::SOLUTIONPUSHBACK);            
			#pragma omp single
            {
                shift = (shift+1)%2;
                char testval = true;
                newChildren = std::find(newChildrenPerThread.begin(), newChildrenPerThread.end(), testval) != newChildrenPerThread.end();
            }
        }
	} // end of OMP parallel
}

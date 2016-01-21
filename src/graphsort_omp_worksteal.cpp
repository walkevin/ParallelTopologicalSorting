#include <omp.h>
#include <algorithm>
#include <cstdlib>

#include "graph.hpp"
#include "analysis.hpp"

using type_threadcount = analysis::type_threadcount;

std::string Graph::getName(){
    return "worksteal";
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


// PRE: 
// POST:	returns unsigned(ceil(n/d)), i.e. round-up division
inline unsigned roundupdiv(unsigned n, unsigned d) {
	return (1 + (n-1)/d);
}


namespace myworksteal {

	class nodePool; // Forward declaration of class

	class threadLocallist {

		public:

			threadLocallist(nodePool& np)
				: np_(np)
				, tid_(-1)
				, locallist_current_fast_()
				, locallist_current_stack_()
				, locallist_next_()
				, solution_local_()
				, currentSyncVal_(0)
			{}

			// TRY TO STEAL A NODE FROM BUSY THREAD
			// PRE: 
			// POST: returns a pointer of a node to work on or nullptr if request was unsuccessful
			inline std::shared_ptr<Node> trySteal() {
				std::shared_ptr<Node> nd = nullptr;
				#pragma omp critical
				{
					if(!locallist_current_stack_.empty()) {
						nd = locallist_current_stack_.back();
						locallist_current_stack_.pop_back();
						#if DEBUG>0 || VERBOSE>1
							std::cout << "\nStealing one node from thread " << tid_;
						#endif // DEBUG>0 || VERBOSE>1
					}
				}
				return nd;
			}

			void work(type_threadcount tid); // implementation below


		private:

			nodePool& np_;
			analysis::type_threadcount tid_;
			Graph::type_nodelist locallist_current_fast_;
			Graph::type_nodelist locallist_current_stack_;
			Graph::type_nodelist locallist_next_;
			Graph::type_nodelist solution_local_;
			Graph::type_size currentSyncVal_;

			friend nodePool;

			// Advances to the next sync value
			// PRE:		locallist_current_* must be empty, the nodes contained in locallist_next_ must have the correct value nsv
			// POST:	swaps the locallist_current_stack_ contains new nodes to work on, locallist_next_ is empty, currenySyncVal_ is set to nsv
			inline void nextSyncVal(Graph::type_size nsv) {
				assert(locallist_current_fast_.empty() && locallist_current_stack_.empty()); // make sure old lists are empty
				assert(locallist_next_.empty() || locallist_next_.front()->getValue()==nsv); // make sure value is set to correct sync value
				currentSyncVal_ = nsv;
				std::swap(locallist_current_stack_,locallist_next_);
				assert(locallist_next_.empty());
			}
			
			// moves nodes from the stack to the fast list critically
			inline void stackToFast() {
				assert(locallist_current_fast_.empty());
				// TODO: critically transfer a "reasonable" portion from stack to fast
				// What is "reasonable"?
				// Experiment with different parameters:
				// - count of threads currently done
				// - ...
				#pragma omp critical 
				{
					// TODO: make better
					// Naive:
					Graph::type_nodelist::const_iterator start = locallist_current_stack_.begin();
					Graph::type_nodelist::const_iterator end = locallist_current_stack_.begin();
					Graph::type_size listsize = locallist_current_stack_.size();
					if(listsize!=0) {
						std::advance(end,roundupdiv(listsize,2));
						locallist_current_fast_.splice(locallist_current_fast_.end(),locallist_current_stack_,start,end);
					}
				}
			}
			
			inline bool noMoreNodes() {
				return (locallist_next_.empty() && locallist_current_fast_.empty() && locallist_current_stack_.empty());
			}


			// used when distributing initial nodes
			// PRE:		ndptr points to valid node
			// POST:	ndptr is inserted into locallist_next_
			inline void insert(Graph::type_nodeptr ndptr) {
				locallist_next_.push_back(ndptr);
			}

			friend std::ostream& operator<<(std::ostream&, threadLocallist&);

	};

	#if DEBUG>0 || VERBOSE>0
	// This is only needed for debugging
	std::ostream& operator<<(std::ostream& os, threadLocallist& tll) {
		os << "\n=================================\nTID: " << tll.tid_;
		os << "\n\tFAST: " << tll.locallist_current_fast_;
		os << "\n\tSTACK: " << tll.locallist_current_stack_;
		os << "\n\tNEXT: " << tll.locallist_next_;
		os << "\n\tSOLUTION: " << tll.solution_local_;
		os << "\n=================================\n";
		return os;
	}
	#endif // DEBUG>0 || VERBOSE>0
	
	
	class nodePool {

		public:
		
			// PRE: must call constructor single threaded
			nodePool(Graph::type_size nThreads, Graph::type_nodelist& sollist, analysis& A)
				: nThreads_(nThreads)
				, globalsolution_(sollist)
				, A_(A)
				, nDoneWithSyncVal_(0)
				, doneWithSyncVal_(nThreads_,0)
				, nodelists_(nThreads_,myworksteal::threadLocallist(*this))
			{
				#if VERBOSE>0
					std::cout << "\n\nInitialized thread-locallists for " << nThreads_ << " threads:\n\n";
				#endif // VERBOSE>0
				for(type_threadcount i=0; i<nThreads_; ++i) {
				#if DEBUG>0 || VERBOSE>0
					#pragma omp critical
					{
						std::cout << nodelists_[i];
					}
				#endif // DEBUG>0 || VERBOSE>0
				}
			}

			// PRE:
			// POST:	doneWithSyncVal_[i] = 0 for all i
			inline void setUndoneAll() {
				std::fill(doneWithSyncVal_.begin(),doneWithSyncVal_.end(),0);
			}

			inline void doneWithStack(type_threadcount i) {
				// #pragma omp critical // TODO: not sure if this is necessary
				doneWithSyncVal_[i] = 1;
			}

			inline void doneWithSyncVal(type_threadcount i) {
				// #pragma omp critical // TODO: not sure if this is necessary
				doneWithSyncVal_[i] = 2;
			}

			inline type_threadcount getNThreads() const {
				return nThreads_;
			}

			inline type_threadcount countDone() { 
				nDoneWithSyncVal_ = std::accumulate(doneWithSyncVal_.begin(),doneWithSyncVal_.end(),type_threadcount(0)); 
				assert(nDoneWithSyncVal_<=2*nThreads_);
				return nDoneWithSyncVal_;
			}

			inline bool allDoneWithStack() const {
				return nDoneWithSyncVal_>=nThreads_;
			}

			inline bool allDoneWithSyncVal() const {
				assert(nDoneWithSyncVal_<=2*nThreads_);
				return nDoneWithSyncVal_>=2*nThreads_;
			}
		

			// PRE:		ndptr is a valid node pointer, i is a valid thread index
			// POST:	ndptr is inserted in the local list of thread tid
			inline void insertNode(Graph::type_nodeptr ndptr, type_threadcount tid) {
				assert(tid<nThreads_);
				nodelists_[tid].insert(ndptr);
			}

			inline bool sortingComplete() {
				for(auto nd : nodelists_) {
					if(!nd.noMoreNodes()) return false;
				}
				return true;
			}

			// spawns threads and synchronizes them
			Graph::type_nodelist& workparallel(Graph& graph) {
				
				// SHARED VARIABLES
				Graph::type_size syncVal = 1;
				bool notdone = true;

				// Spawn OMP threads
				#pragma omp parallel shared(syncVal,notdone)
				{

					// TODO: check how this runs on cluster:
					// this may need to be removed - maybe nThreads > omp_get_num_threads
					#pragma omp single
					assert(nThreads_==omp_get_num_threads()); 

					// THREAD PRIVATE VARIABLES
					const int threadID = omp_get_thread_num();
		
					#pragma omp barrier

					do {

						#pragma omp critical 
						nodelists_[threadID].nextSyncVal(syncVal);
						#pragma omp barrier
						
						#if VERBOSE>0
							#pragma omp single
							std::cout << "\nCurrent syncVal = " << syncVal;
						#endif // VERBOSE>0
	
						nodelists_[threadID].work(threadID);
						#pragma omp barrier

						#pragma omp single
						notdone = !sortingComplete();
						
						#pragma omp single nowait
						setUndoneAll();
						
						#pragma omp single nowait
						++syncVal;

					} while(notdone);
				
				} // end of OMP parallel
				#pragma omp single
				graph.setDepth(syncVal);

			}

			inline Graph::type_nodeptr tryStealFrom(type_threadcount tid) {
				return nodelists_[tid].trySteal();
			}

			analysis& A_;

		private:
			const type_threadcount nThreads_;
			Graph::type_nodelist& globalsolution_;
			type_threadcount nDoneWithSyncVal_;
			std::vector<type_threadcount> doneWithSyncVal_;
			std::vector<myworksteal::threadLocallist> nodelists_;

		friend std::ostream& operator<<(std::ostream&, nodePool&);
		friend class threadLocallist;
	};

	#if DEBUG>0 || VERBOSE>0
	std::ostream& operator<<(std::ostream& os, nodePool& ndp) {
		for(type_threadcount i=0; i<ndp.nThreads_; ++i) {
			std::cout << "\n\nCurrent state of ThreadPool:\n";
			std::cout << ndp.nodelists_[i];
		}
	}
	#endif // DEBUG>0 || VERBOSE>0

	
	
	// functions with dependencies on pool go here

	void threadLocallist::work(type_threadcount tid) {

		// make sure this is always called with the same thread-id
		assert(tid_==tid || tid_==-1);
		tid_ = tid;

		assert(locallist_next_.empty());

		Graph::type_nodeptr parent;
		Graph::type_nodeptr child;
		Graph::type_size childcount = 0;

		#pragma omp barrier // make sure everything is set up alright
	

		Graph::type_size currentvalue;
		do {
			while(!(locallist_current_stack_.empty() && locallist_current_fast_.empty())) {
				while(!locallist_current_fast_.empty()) {

					parent = locallist_current_fast_.front();
					assert(parent->getValue() == currentSyncVal_); // TODO: this can be removed once working
				
					solution_local_.push_back(parent); // put node in solution
					locallist_current_fast_.pop_front(); // remove current node - already visited
					
					currentvalue = currentSyncVal_+1; // increase value for child nodes
					childcount = parent->getChildCount();
					
					bool flag;
					for(Graph::type_size c=0; c<childcount; ++c) {
						child = parent->getChild(c);

						// Checking if last parent trying to update
						flag = child->requestValueUpdate(); // IMPORTANT: control atomicity using OPTIMISTIC flag
						
						if(flag) { // last parent checking child
							locallist_next_.push_back(child); // add child node at end of queue
							child->setValue(currentvalue); // set value of child node to parentvalue
						} 
				
					}
                    np_.A_.incrementProcessedEdges(tid_, childcount);
					np_.A_.incrementProcessedNodes(tid_);

				}
				stackToFast();
			}
	
			np_.doneWithStack(tid_);
			np_.countDone();

			if(np_.allDoneWithStack()) { // all threads are done with stack - no point in trying to steal
				break;
			} else { // steal work while others are still busy 
				analysis::type_threadcount stealindex = rand()%np_.getNThreads();
				Graph::type_nodeptr newnode = np_.tryStealFrom(stealindex);
				if(newnode!=nullptr) {
					locallist_current_fast_.push_back(newnode);
				}
			}

		} while(!np_.allDoneWithSyncVal());

		np_.doneWithSyncVal(tid_);

		#pragma omp barrier

		// Collect local lists in global list
		gatherlist(np_.globalsolution_,solution_local_,tid_);

	}

}



void Graph::topSort() {

	myworksteal::nodePool nodepool(this->A_.nThreads_,this->solution_,A_);

	// Sorting Magic happens here

	// Start: currentnodes = root nodes 
	type_threadcount i=0;
	for(auto nd : nodes_) {
		if(nd->getValue()==1) {
			nodepool.insertNode(nd,i);
			++i;
			i %= nodepool.getNThreads();
		}
	}

	#if DEBUG>0 || VERBOSE>0
	std::cout << nodepool;
	#endif // DEBUG>0 || VERBOSE>0

	nodepool.workparallel(*this);

}

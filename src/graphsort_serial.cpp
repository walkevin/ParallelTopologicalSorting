#include <omp.h>

#include "graph.hpp"
#include "analysis.hpp"

std::string Graph::getName(){
    return "serial";
}

void Graph::topSort() {
	
	// Sorting Magic happens here
	std::list<std::shared_ptr<Node> > currentnodes;
	
	std::shared_ptr<Node> parent;
	std::shared_ptr<Node> child;
	unsigned childcount = 0;
	unsigned currentvalue = 0;

	// Initialize with root nodes
	for(unsigned i=0; i<N_; ++i) {
		if(nodes_[i]->getValue()==1) currentnodes.push_back(nodes_[i]);
	}

	while(!currentnodes.empty()) {

		parent = currentnodes.front();
		currentvalue = parent->getValue();

		solution_.push_back(parent); // IMPORTANT: this must be atomic
		currentnodes.pop_front(); // remove current node - already visited

		++currentvalue; // increase value for child nodes
		childcount = parent->getChildCount();

		bool flag;
		for(unsigned c=0; c<childcount; ++c) {
			child = parent->getChild(c);

			// Checking if last parent trying to update
			flag = child->requestValueUpdate(); // IMPORTANT: this must be atomic
			
			if(flag) { // last parent checking child
				currentnodes.push_back(child); // add child node at end of queue
				child->setValue(currentvalue); // set value of child node to parentvalue
			} 
	
		}
	}

}

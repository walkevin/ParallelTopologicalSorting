#include "node.hpp"

using type_value = Node::type_value;
using type_nodecontainer = Node::type_nodecontainer;


void Node::addChild(std::shared_ptr<Node> child) {
	childnodes_.push_back(child); // add child to container
	child->parcount_++; // increase parent counter of child
	++childcount_;
	child->setValue(0); // set value to 0 - child is not a root anymore
}

bool Node::hasChild(std::shared_ptr<Node> childCandidate) {
	for(auto child : childnodes_) {
		if(child == childCandidate)
			return true;
	}
	return false;
}

#ifndef NODE_HPP
#define NODE_HPP

#include <iostream>
#include <list>
#include <vector>
#include <cassert>
#include <memory>


class Node {

	public:

		using type_index = unsigned;
		using type_value = unsigned;
		using type_nodecontainer = std::vector<std::shared_ptr<Node> >;

		Node(type_index id)
			: id_(id)
			, v_(1) // value = 1 by default
			, childcount_(0)
			, parcount_(0)
			, childnodes_(type_nodecontainer(childcount_))
            , taken_(false)
		{
			// NOTE: value = 1 indicates that a node is a source node
			// (no other nodes pointing to it)
			// by default every node that is constructed is a source node
		}

		void addChild(std::shared_ptr<Node> child);
		bool hasChild(std::shared_ptr<Node> childCandidate);

		inline type_value getValue() const {
			return v_;
		}
		
		inline type_value getID() const {
			return id_;
		}

		inline void setValue(type_value r) {
			v_ = r;
		}

		inline std::shared_ptr<Node> getChild(type_index i) {
			assert(i<childcount_);
			return childnodes_[i];
		}

		inline unsigned getChildCount() const {
			return childcount_;
		}

#if OPTIMISTIC == 1
		inline bool requestValueUpdate() {
			#pragma omp atomic
			--parcount_;
			assert(parcount_>=0);
			return (parcount_ == 0);
		}
#elif OPTIMISTIC == 2
		inline bool requestValueUpdate() {
			#pragma omp atomic
			--parcount_;
			assert(parcount_>=0);
            bool swapped = false;
            if(parcount_ == 0)
                swapped = __sync_bool_compare_and_swap(&taken_, false, true);
			return swapped;
		}
#else
		inline bool requestValueUpdate() {
			bool lastone;
			#pragma omp critical
			{
			--parcount_;
			assert(parcount_>=0);
			lastone = (parcount_ == 0);
			}
			return lastone;
		}
#endif // OPTIMISTIC


	private:

		const type_index id_;
		unsigned childcount_;
		type_value v_;
		type_nodecontainer childnodes_;
		
	protected:
		unsigned parcount_;
        bool taken_;

};

#endif // NODE_HPP

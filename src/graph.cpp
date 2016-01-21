#include "graph.hpp"

#include <cassert>
#include <string>
#include <cstdio>
#include <list>
#include <vector>
#include <random>
#include <functional>
#include <numeric>
#include <memory>
#include <omp.h>

using type_size = Graph::type_size;

void Graph::connect(GRAPH_TYPE type, double edgeFillDegree, double p, double q, int nChains) {
	
	std::cout << "\nConnection Mode:\t";

	Node nd = Node(2);

	switch(type) {

		case PAPER: // Construct simple example graph from paper
			assert(N_==9);
			nodes_[0]->addChild(nodes_[2]);
			nodes_[2]->addChild(nodes_[6]);
			nodes_[6]->addChild(nodes_[3]);
			nodes_[6]->addChild(nodes_[4]);
			nodes_[3]->addChild(nodes_[5]);
			nodes_[4]->addChild(nodes_[7]);
			nodes_[7]->addChild(nodes_[5]);
			nodes_[1]->addChild(nodes_[7]);
			nodes_[8]->addChild(nodes_[1]);
			nodes_[8]->addChild(nodes_[4]);
            graphName_ = "PAPER";
			std::cout << "PAPER";
			break;

        case RANDOM_LIN:
        {
			// Specify (roughly) number of edges            
            int nEdges = N_ * edgeFillDegree;
            connectRandom(nEdges);
            graphName_ = "RANDOMLIN";
            std::cout << "RANDOM_LIN (target fill degree: " << static_cast<double>(nEdges) / (0.5 * N_ * (N_-1)) << ")";

            break;
        }
        
		case RANDOM_QUAD:
		{
			// Specify (roughly) number of edges
            int nEdges = N_ * (N_ - 1) * 0.5 * edgeFillDegree;
            connectRandom(nEdges);
            graphName_ = "RANDOMQUAD";
			std::cout << "RANDOM_QUAD (target fill degree: " << edgeFillDegree << ")";
			break;
		}

        case SOFTWARE:
        {
            // See Musco 2014, A Generative Model of Software Dependency Graphs to Better Understand Software Evolution
			assert(p >= 0. && p <= 1.);
            assert(q >= 0. && q <= 1.);
           
			// Prepare the random number generator
			const int seed = 42;
			std::mt19937 gen(seed);
			std::uniform_real_distribution<double> dis(0, 1);
			auto rnd = std::bind(dis, gen);

			for(type_size i = 1; i < N_; i++){
                int n_insertedNodes = i;
                // choose a random node, which has already been inserted
                auto r_node = static_cast<int>(std::round(rnd() * (n_insertedNodes-1)));
                
                // with probability p attach current to node random node and all of its children
                auto r_p = rnd();
                if(r_p < p){
                    nodes_[i]->addChild(nodes_[r_node]);
                    
                    for(size_t c = 0; c < nodes_[r_node]->getChildCount(); ++c){
                        nodes_[i]->addChild(nodes_[r_node]->getChild(c));
                    }
                    
                    // with probability q attach another random node
                    auto r_q = rnd();
                    if(r_q < q){ 
                        auto r_node2 = static_cast<int>(std::round(rnd() * (n_insertedNodes-1)));
                        if(r_node != r_node2){
                            if(!nodes_[i]->hasChild(nodes_[r_node2])){
                                nodes_[i]->addChild(nodes_[r_node2]);
                            }
                            for(size_t c = 0; c < nodes_[r_node2]->getChildCount(); ++c){
                                if(!nodes_[i]->hasChild(nodes_[r_node2])){
                                    nodes_[i]->addChild(nodes_[r_node2]->getChild(c));
                                }
                            }
                        }
                    }
                    
                }
                // with probability 1-p attach random node to current node
                else{ 
                    nodes_[r_node]->addChild(nodes_[i]);
                }
            }
            graphName_ = "SOFTWARE";            
			std::cout << "SOFTWARE (attach probability (p): " << p << " attached probability (1-p): " << 1-p << ", double attach probability (q|p=true): " << q << ", )";
			break;
        }
        
        case CHAIN:
        {
            for(int i = 0; i < N_ -1; ++i){
                nodes_[i]->addChild(nodes_[i+1]);
            }
            graphName_ = "CHAIN";
            std::cout << "CHAIN\n";
            break;
        }
        
        case MULTICHAIN:
        {
            assert(nChains <= N_);
            for(int i = 0; i < N_ - 1; ++i){
                if((i+1) % (N_ / nChains) == 0)
                    continue;
                nodes_[i]->addChild(nodes_[i+1]);
            }
            graphName_ = "MULTICHAIN";
            std::cout << "MULTICHAIN (number of chains: " << nChains << ")";
            break;
        }
        
		default:
			std::cout << "\nERROR:\tInvalid connection index - no connections added\n";

	}


    assert(graphName_ != "");
	nEdges_ = countEdges();

	std::cout << "\n(Nodes: " << N_ << ", Edges: " << nEdges_ << ", FillDegree: " << static_cast<double>(nEdges_) / (0.5 * N_ * (N_-1)) << ")";
	std::cout << "\n";

}

void Graph::connectRandom(int nEdges){
    assert(nEdges <= N_ * (N_ - 1) * 0.5);
    // Create random order of nodes
    std::vector<unsigned> order(N_);
    std::iota(order.begin(), order.end(), 0);
    const int seed2 = 55;
    std::mt19937 gen2(seed2);
    std::shuffle(order.begin(), order.end(), gen2);

    // Prepare the random number generator
    const int seed = 42;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<type_size> dis(0, N_-1);
    auto rnd = std::bind(dis, gen);

    for(type_size i = 0; i < nEdges; i++){
        auto rn0 = rnd();
        auto rn1 = rnd();
        
        if(rn0 == rn1)
            continue;

        // Node with lower order always points to node with higher order so as to avoid circles
        auto pointerNode = rn0;
        auto pointeeNode = rn1;
        if(order[rn0] > order[rn1]){
            pointerNode = rn1;
            pointeeNode = rn0;
        }

        if(!nodes_[pointerNode]->hasChild(nodes_[pointeeNode])){
            nodes_[pointerNode]->addChild(nodes_[pointeeNode]);
        }
    }
}

type_size Graph::countEdges() {
    type_size nEdges = 0;
	for(auto ndptr : nodes_) {
		nEdges += ndptr->getChildCount();
	}
    return nEdges;
}

std::vector<type_size> Graph::getChildrenQuantiles() {
    std::vector<type_size> quantiles(5, 0);
    std::vector<type_size> n_childrenPerNode;
    for(auto ndptr : nodes_) {
        n_childrenPerNode.push_back(ndptr->getChildCount());
    }
    std::sort(n_childrenPerNode.begin(), n_childrenPerNode.end());
    quantiles[0] = n_childrenPerNode[0];
    quantiles[1] = n_childrenPerNode[N_-1 / 4];
    quantiles[2] = n_childrenPerNode[N_ / 2];
    quantiles[3] = n_childrenPerNode[3 * N_ / 4];
    quantiles[4] = n_childrenPerNode[N_ - 1];
    
    return quantiles;
}

bool Graph::checkCorrect(bool verbose) {
	
    std::cout << "\nChecking solution correctness...\n";
	bool correct = true;

    analysis::type_error errorCode = 0;
    // 1. check length of solution
    if(solution_.size() != nodes_.size()){
        correct = false;
        errorCode += 1;
        if(verbose)
            std::cout << "ERROR: Size of solution is " << solution_.size() << ", but should be " << nodes_.size() << "\n";
    }

    // retrieve the order of each node from solution
    std::vector<size_t> nodeOrders(N_);
    std::vector<size_t> checkNodes(N_, 0);
    size_t cnt = 0;
    for(auto it = solution_.begin(); it != solution_.end(); ++it){
        size_t nodeId = (*it)->getID();
        nodeOrders[nodeId] = cnt;
        checkNodes[nodeId]++;
        ++cnt;
    }
    
    // 2. check that every node occurs exactly once in the solution
    for(size_t i = 0; i < N_; ++i){
        if(checkNodes[i] != 1){
            correct = false;
            errorCode += 2;
            if(verbose)
                std::cout << "ERROR: Node #" << nodes_[i]->getID() << " occurs " << checkNodes[i] << " times, but should occur exactly once.\n";
        }
    }
    
    // 3. for each (parent) node, check that each of their children has a higher sorting index
    for(size_t i = 0; i < N_; ++i){
        auto parent = nodes_[i];
        auto parentId = parent->getID();
        size_t childcount = parent->getChildCount();
        
        for(size_t k = 0; k < childcount; ++k){
            size_t childId = parent->getChild(k)->getID();
            if(nodeOrders[parentId] > nodeOrders[childId]){
                correct = false;
                errorCode += 4;
                if(verbose)
                    std::cout << "ERROR: Node #" << parentId << " should have lower index than node #" << childId << "\n";
            }
        }
    }

    A_.errorCode_ = errorCode;
	if(correct) {
		std::cout << "\n\033[1;32mOK\033[0m: VALID TOPOLOGICAL SORTING.\n\n";
	} else {
		std::cout << "\n\033[1;31mERROR: INVALID TOPOLOCIGAL SORTING!\033[0m\n\n";
	}

	#if VERBOSE>=2
	if(N_<=400) printNodeInfo();
	#elif VERBOSE>0
	if(N_<=1000) printSolution();
	#endif

	return correct;
}


#if DEBUG>0 || VERBOSE>0
// Overloading output operator of nodelist
// Can be useful for debugging
std::ostream& operator<<(std::ostream& os, Graph::type_nodelist& ls) {
	os << "\n[ ";
	for(auto ndptr : ls) {
		os << ndptr->getID() << "(" << ndptr->getValue() << ")" << " ";
	}
	os << "]\n";
	return os;
}
#endif // DEBUG>0 || VERBOSE>0

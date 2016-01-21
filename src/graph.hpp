#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <omp.h>

#include "node.hpp"
#include "analysis.hpp"


class Graph {

	public:

		enum GRAPH_TYPE {PAPER, RANDOM_LIN, RANDOM_QUAD, SOFTWARE, CHAIN, MULTICHAIN};

		using type_nodeptr = std::shared_ptr<Node>;
		using type_nodearray = std::vector<type_nodeptr>;
        using type_nodelist = std::list<type_nodeptr>;
        using type_solution = type_nodelist; // NB: I would prefer type_nodelist over type_solution - more generic (not all nodelists are solutions, for example currentnodes)
        using type_size = analysis::type_size;
        
        explicit Graph(unsigned N)
			:	N_(N)
			,	nEdges_(0)
			,	depth_(0)
			,	nodes_(type_nodearray(N_))
			,	A_()
		{
			std::cout << "DEBUG = " << DEBUG << "\tVERBOSE = " << VERBOSE << "\tOPTIMISTIC = " << OPTIMISTIC << "\tENABLE_ANALYSIS = " << ENABLE_ANALYSIS << "\n\n";
			std::cout << "Initializing graph of size " << N_ << "...\n";
			for(unsigned i=0; i<N_; ++i) {
				nodes_[i] = std::make_shared<Node>(i);
			}
		}

		analysis::type_time time_topSort() {
            
            // Store Meta-information for analysis
            A_.algorithmName_ = getName();
            A_.nNodes_ = N_;
            A_.nEdges_ = nEdges_;
            A_.graphName_ = graphName_;
            A_.nChildrenQuantiles_ = getChildrenQuantiles();
            
            // Start topological sorting
			A_.starttotaltiming();
			this->topSort();
			A_.stoptotaltiming();
            A_.depth_ = depth_;
			std::cout << "\n\nMaximum Diameter: " << depth_;
			std::cout << "\n\n\tSorting completed in:\t" << std::setprecision(8) << std::fixed << A_.time_Total_ << " sec\n\n";
			return A_.time_Total_;
		}
        void topSort();
        std::string getName();
        
        /** \brief Connects nodes (= creates edges) according to a GRAPH_TYPE.
         *  \param edgeFillDegree  For GRAPH_TYPE=RANDOM_LIN, edgeFillDegree = 1 creates exactly as many edges as nodes.
         *                         For GRAPH_TYPE=RANDOM_QUAD, edgeFillDegree = 1 creates all possible edges.
         *  \param p  \in[0,1] For GRAPH_TYPE=SOFTWARE. Details see Musco et al.
         *  \param q  \in[0,1] For GRAPH_TYPE=SOFTWARE. Details see Musco et al.
         *  \param nChains  For GRAPH_TYPE=MULTICHAINS. Creates nChains many chains. nChains must be lower than number of nodes.
         */
		void connect(GRAPH_TYPE, double edgeFillDegree = .3, double p = .5, double q = .7, int nChains = 100);
		type_size countEdges();
        /** \brief Returns the 0 (aka min), 25, 50 (aka median), 75 and 100 (aka max) quantile of the number of children of each node.
         */  
        std::vector<type_size> getChildrenQuantiles();
        bool checkCorrect(bool verbose);
        type_solution getSolution();
        
        // Print and doc methods (graphdoc.cpp)
		void printNodeInfo();
        void printSolution();
		void viz(std::string) const;
        void dumpXmlAnalysis(std::string relativeDir);
        void setDepth(type_size d) {
        	depth_ = d;
        }
        
        type_solution solution_; // TODO: needed to make this public, maybe with better design it can be made private again

	protected:

        void connectRandom(int nEdges);
		type_size N_; // size of graph, == W
		type_size nEdges_; // number of edges
        type_size depth_; // depth of graph, == D
        std::string graphName_;
		type_nodearray nodes_;
        analysis A_;

};


#if DEBUG>0 || VERBOSE>0
// Overloading output operator of nodelist
// Can be useful for debugging
std::ostream& operator<<(std::ostream& os, Graph::type_nodelist& ls);
#endif // DEBUG>0 || VERBOSE>0


#endif // GRAPH_HPP

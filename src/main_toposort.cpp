#include <iostream>
#include <omp.h>
#include <string>

#include "graph.hpp"
#include "analysis.hpp"

int main(int argc, char* argv[]) {
    if(argc == 2 && std::string(argv[1]) == "--help"){
        std::cout << "Usage: ./toposort_xyz.exe [graphType = s [,N=5000 [,destDir=results [,edgeFillDegree = 2.7 [,p = 0.5, q = 0.7 [,nChains = 100]]]]]]" << std::endl;
        std::cout << "Graph Types: t: Test graphs (Paper and small Random)\ts: Software\tr: Random \tc: Chain\tm: Mulitchain" << std::endl;
        return 0;
    }
    // Standard values
    char graphType = 's';
    unsigned N = 500000;
    std::string out_dir = "results/";
    double edgeFillDegree = 2.7;
    double p = 0.5;
    double q = 0.7;
    int nChains = 100;
    
    // Read in command-line overrides
    int cnt_arg = 1;
    if(argc >= ++cnt_arg)
        graphType = argv[cnt_arg-1][0];
    if(argc >= ++cnt_arg)
        N = std::stoi(argv[cnt_arg-1]);
    if(argc >= ++cnt_arg)
        out_dir = argv[cnt_arg-1];
    if(argc >= ++cnt_arg)
        edgeFillDegree = std::stod(argv[cnt_arg-1]);
    if(argc >= ++cnt_arg)
        p = std::stod(argv[cnt_arg-1]);
    if(argc >= ++cnt_arg)
        q = std::stod(argv[cnt_arg-1]);
    if(argc >= ++cnt_arg)
        nChains = std::stoi(argv[cnt_arg-1]);    
        
	std::string visualbarrier(70,'=');
	visualbarrier = "\n\n\n" + visualbarrier + "\n\n";

    switch (graphType){
		case 'p':
		{
            // PAPER GRAPH - Testing
            std::cout << visualbarrier;
            Graph testgraph_paper(9);
            testgraph_paper.connect(Graph::PAPER); // Constructing graph from paper
            testgraph_paper.time_topSort();
            testgraph_paper.checkCorrect(true);
            testgraph_paper.viz("paper");
            break;
		}
        case 't':
        {
            // PAPER GRAPH - Testing
            std::cout << visualbarrier;
            Graph testgraph_paper(9);
            testgraph_paper.connect(Graph::PAPER); // Constructing graph from paper
            testgraph_paper.time_topSort();
            testgraph_paper.checkCorrect(true);
            testgraph_paper.viz("paper");

            // RANDOM GRAPH - SMALL
            std::cout << visualbarrier;
            Graph testgraph_random_small(40);
            testgraph_random_small.connect(Graph::RANDOM_LIN, edgeFillDegree);
            testgraph_random_small.time_topSort();
            testgraph_random_small.checkCorrect(false);
            testgraph_random_small.viz("random_lin");
            break;
        }
        case 'r':
        {
            // RANDOM GRAPH - MEDIUM
            std::cout << visualbarrier;
            Graph testgraph_random(N);
            testgraph_random.connect(Graph::RANDOM_LIN, edgeFillDegree);
            testgraph_random.time_topSort();
            testgraph_random.checkCorrect(false);
            testgraph_random.dumpXmlAnalysis(out_dir);
            break;
        }
    
        case 's':
        {
            // SOFTWARE GRAPH - MEDIUM
            std::cout << visualbarrier;
            Graph softwaregraph(N);
            softwaregraph.connect(Graph::SOFTWARE, 0., p, q);
            softwaregraph.time_topSort();
            softwaregraph.checkCorrect(false);
            softwaregraph.dumpXmlAnalysis(out_dir);
            break;
        }
        
        case 'c':
        {
            // CHAIN GRAPH - MEDIUM
            std::cout << visualbarrier;
            Graph testgraph_chain(N);
            testgraph_chain.connect(Graph::CHAIN);
            testgraph_chain.time_topSort();
            testgraph_chain.checkCorrect(false);
            testgraph_chain.dumpXmlAnalysis(out_dir);
            break;
        }
        
        case 'm':
        {
            // MULTICHAIN GRAPH - MEDIUM
            std::cout << visualbarrier;
            Graph testgraph_multichain(N);
            testgraph_multichain.connect(Graph::MULTICHAIN, 0., 0., 0., nChains);
            testgraph_multichain.time_topSort();
            testgraph_multichain.checkCorrect(false);
            testgraph_multichain.dumpXmlAnalysis(out_dir);
            break;
        }
        
        default:
            std::cout << "Unknown Graph Type " << graphType << std::endl;
            std::cout << "Graph Types: t: Test graphs (Paper and small Random)\ts: Software\tr: Random \tc: Chain\tm: Mulitchain" << std::endl;
    }

	std::cout << visualbarrier;
    
	return 0;

}

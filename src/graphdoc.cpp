#include "graph.hpp"

#include <cassert>
#include <string>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <list>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/filesystem/convenience.hpp>

#ifndef CMAKE_SOURCE_DIR
#define CMAKE_SOURCE_DIR "."
#endif


// Print Node Info to console
void Graph::printNodeInfo() {
	for(auto ndptr : nodes_) {
		std::cout << "ID: " << ndptr->getID() << "\tV: " << ndptr->getValue() << "\n";
	}
	std::cout << "\n";
}

void Graph::printSolution() {
    std::cout << "\nSolution (Node IDs)" << std::endl;
    for(auto elem : solution_){
        std::cout << elem->getID() << " ";
    }
    std::cout << std::endl;
}

// Create graphviz file for drawing graph
void Graph::viz(std::string graphfilename) const {

	// Don't create huge graphviz files
	if(N_>200) return;

	graphfilename.append(".gv");

	FILE* outfile_ptr;
	
	std::string dirstr = std::string(CMAKE_SOURCE_DIR) + "/graph_output/";
	std::string path = dirstr + graphfilename;

	// Boost: create directory if it doesn't exist yet
	boost::filesystem::path dir(dirstr.c_str());
	if(boost::filesystem::create_directories(dir)) {
		std::cout << "\nDirectory " << dir << " created\n";
	}
	
	outfile_ptr = fopen(path.c_str(),"w");
	if(outfile_ptr != NULL) {
		fprintf(outfile_ptr,"# Visualization of Graph %s, size=%u\n\n",path.c_str(),N_);
		fprintf(outfile_ptr,"digraph g {\n");
		// fprintf(outfile_ptr,"\tranksep=1;\n");
		// fprintf(outfile_ptr,"\tratio=auto;\n");
	
		fprintf(outfile_ptr,"\n\t#TITLE\n\tlabelloc=\"t\";\n\tlabel=\"type=%s, size=%u;\"\n",graphfilename.c_str(),N_);

		fprintf(outfile_ptr,"\n\t# NODES\n");
		unsigned maxv = 0;
		for(unsigned n=0; n<N_; ++n) {
			unsigned v = nodes_[n]->getValue();
			maxv = std::max(v,maxv);
		}
		std::cout << "maxv = " << maxv;

		// Constructing color string for node
		std::stringstream stream;

		for(unsigned n=0; n<N_; ++n) {
			unsigned v = nodes_[n]->getValue();
			stream << "#";
			stream << std::setfill ('0') << std::setw(2) << std::hex << 0x00; // unsigned((250.*v)/maxv);
			stream << std::setfill ('0') << std::setw(2) << std::hex << 0x00; // unsigned((250.*v)/maxv);
			stream << std::setfill ('0') << std::setw(2) << std::hex << unsigned((30+(220.*v))/maxv);
			std::string color( stream.str() );
			stream.str(std::string()); // clear string stream
			// std::cout << "\n" << color;
			fprintf(outfile_ptr,"\tN%02u [ label=\"%2u\"color=\"#000000\", fillcolor=\"%s\", fontcolor=white, fontname=\"Courier:italic\", shape=\"circle\", style=\"filled,solid\" ];\n",n,v,color.c_str());
		}

		fprintf(outfile_ptr,"\n\t# EDGES\n");
		unsigned cc;
		for(unsigned n=0; n<N_; ++n) {
			cc = nodes_[n]->getChildCount(); // number of children of current node
			for(unsigned c=0; c<cc; ++c) { // draw arrow to all children
				fprintf(outfile_ptr,"\tN%02u -> N%02u [ penwidth=2, style=\"solid\", color=\"#000000\" ];\n",n,(nodes_[n]->getChild(c))->getID());
			}
		}
		
		fprintf(outfile_ptr,"}");
	} else {
		std::cerr << "\n\nPath doesn't exist: " << path << std::endl;
	}

	fclose(outfile_ptr);
	std::cout << "\n\nGraph file created as " << graphfilename << std::endl;
}

void Graph::dumpXmlAnalysis(std::string relativeDir){
    A_.xmlAnalysis(relativeDir);
}

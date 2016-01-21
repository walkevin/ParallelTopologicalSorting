#include "analysis.hpp"


#include <ctime>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <random>
#include <limits.h>
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/convenience.hpp>


std::string analysis::suggestBaseFilename(){
    std::string sep = "_";
    #ifdef OPTIMISTIC
    std::string opt = std::to_string(OPTIMISTIC);
    #else
    std::string opt = "0";
    #endif
    
    #ifdef ENABLE_ANALYSIS
    std::string an = std::to_string(ENABLE_ANALYSIS);
    #else
    std::string an = "0";
    #endif

    std::string env_host;
    char hostname[HOST_NAME_MAX];
    int result = gethostname(hostname, HOST_NAME_MAX);
    if (result)
      env_host = "Unk";
    else
      env_host = std::string(hostname);


    std::stringstream ss;
           ss << algorithmName_
           << sep << "opt" << opt
           << sep << "an" << an
           << sep << "t" << nThreads_
           << sep << "p" << nProcs_
           << sep << graphName_
           << sep << "n" << nNodes_
           << sep << "e" << nEdges_
           << sep << env_host
           ;
    return ss.str();
}

bool analysis::xmlAnalysis(std::string relativeDir){
    std::string env_host;
    char hostname[HOST_NAME_MAX];
    int result = gethostname(hostname, HOST_NAME_MAX);
    if (result)
      env_host = "Unk";
    else
      env_host = std::string(hostname);
    

	// Boost: create directory if it doesn't exist yet
	boost::filesystem::path boost_reldir(relativeDir.c_str());
	if(boost::filesystem::create_directories(boost_reldir)) {
		std::cout << "\nDirectory " << boost_reldir << " created\n";
	}



    std::stringstream output;
    output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    output << "<measurements>\n";
    output << "\t<measurement>\n";
    output << "\t\t<date>" << std::time(nullptr) << "</date>\n";
    output << "\t\t<errorCode>" << errorCode_ << "</errorCode>\n";
    output << "\t\t<numberOfThreads>" << nThreads_ << "</numberOfThreads>\n";
    output << "\t\t<processors>" << nProcs_ << "</processors>\n";
    output << "\t\t<hostname>" << env_host << "</hostname>\n";
    output << "\t\t<totalTime>" << time_Total_ << "</totalTime>\n";
    output << "\t\t<algorithm>" << algorithmName_ << "</algorithm>\n";
    
    #if ENABLE_ANALYSIS == 1
    // in-depth analysis
    output << "\t\t<threads>\n";
    for(size_t i = 0; i < nThreads_; ++i){
        output << "\t\t\t<thread>\n";
        output << "\t\t\t<id>" << i << "</id>\n";
        output << "\t\t\t<timings>\n";
        output << "\t\t\t\t<timing name=\"barrier\">" << timings_[BARRIER][i] << "</timing>\n";
        output << "\t\t\t\t<timing name=\"criticalPushBack\">" << timings_[SOLUTIONPUSHBACK][i] << "</timing>\n";
        output << "\t\t\t\t<timing name=\"criticalRequestValueUpdate\">" << timings_[REQUESTVALUEUPDATE][i] << "</timing>\n";
        output << "\t\t\t\t<timing name=\"currentGather\">" << timings_[CURRENTGATHER][i] << "</timing>\n";
        output << "\t\t\t\t<timing name=\"currentScatter\">" << timings_[CURRENTSCATTER][i] << "</timing>\n";
        output << "\t\t\t</timings>\n";
        output << "\t\t\t<processedNodes>" << count_ProcessedNodes_[i] << "</processedNodes>\n";
        output << "\t\t\t<processedEdges>" << count_ProcessedEdges_[i] << "</processedEdges>\n";
        output << "\t\t\t</thread>\n";
    }
    output << "\t\t</threads>\n";
    #endif
    
    output << "\t\t<graph>\n";
    output << "\t\t\t<type>" << graphName_ << "</type>\n";
    output << "\t\t\t<numberOfNodes>" << nNodes_ << "</numberOfNodes>\n";
    output << "\t\t\t<numberOfEdges>" << nEdges_ << "</numberOfEdges>\n";
    output << "\t\t\t<depth>" << depth_ << "</depth>\n";
    
    #if ENABLE_ANALYSIS == 1
    // Quantiles of number of children
    if(nChildrenQuantiles_.size() == 5){
        output << "\t\t\t<numberOfChildren>\n";
        output << "\t\t\t\t<q0>" << nChildrenQuantiles_[0] << "</q0>\n";
        output << "\t\t\t\t<q25>" << nChildrenQuantiles_[1] << "</q25>\n";
        output << "\t\t\t\t<q50>" << nChildrenQuantiles_[2] << "</q50>\n";
        output << "\t\t\t\t<q75>" << nChildrenQuantiles_[3] << "</q75>\n";
        output << "\t\t\t\t<q100>" << nChildrenQuantiles_[4] << "</q100>\n";
        output << "\t\t\t</numberOfChildren>\n";
    }
    
    // Quantiles of front size
    if(frontSizes_.size() != 0){
        std::sort(frontSizes_.begin(), frontSizes_.end());
        output << "\t\t\t<frontSizes>\n";
        int n_quantiles = 10;
        for(int i = 0; i < n_quantiles; ++i){
            output << "\t\t\t\t<q" << i*n_quantiles << ">" << frontSizes_[i * frontSizes_.size() / n_quantiles] << "</q" << i*n_quantiles << ">\n";
        }
        output << "\t\t\t\t<q100>" << frontSizes_.back() << "</q100>\n";
        output << "\t\t\t</frontSizes>\n";
    }
    #endif
    
    output << "\t\t</graph>\n";
    #if OPTIMISTIC==1 || OPTIMISTIC==2
    output << "\t\t<optimistic>true</optimistic>\n";
    #else
    output << "\t\t<optimistic>false</optimistic>\n";
    #endif
    
    #if ENABLE_ANALYSIS==1
    output << "\t\t<enableAnalysis>true</enableAnalysis>\n";
    #else
    output << "\t\t<enableAnalysis>false</enableAnalysis>\n";
    #endif
    
    output << "\t</measurement>\n";
    output << "</measurements>\n";

    //Check if relativeDir has delimiter
    if(relativeDir == ""){
        relativeDir = "./";
    }
    else if(relativeDir.back() != '/'){
        relativeDir += "/";
    }
    
    std::string baseFilename = relativeDir + suggestBaseFilename();
    std::string filename = baseFilename + ".xml";  
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 999);

    //Check if file exists
    while(true){
        std::ifstream infile(filename);
        bool exists = infile.is_open();
        if(!exists)
            break;
        infile.close();
        std::string rn = std::to_string(dis(gen));
        filename = baseFilename + "." + rn + ".xml";
    }
    std::ofstream f(filename);
    if(f){
        f << output.str();
        f.close();
    }
    else{
        std::cerr << "Could not open file " << filename << std::endl;
        return false;
    }
    
    std::cout << "Analysis written to: " << filename << std::endl;
    return true;
}

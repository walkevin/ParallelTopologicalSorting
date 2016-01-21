#ifndef ANALYSIS_HPP
#define ANALYSIS_HPP

#include "rdtsc_timer.hpp"
#include <omp.h>
#include <cassert>
#include <vector>

#if ENABLE_ANALYSIS == 1

#include <map>
#include <ostream>


struct analysis {

	// TYPES AND VARIABLES
	enum timecat {BARRIER,SOLUTIONPUSHBACK,REQUESTVALUEUPDATE,CURRENTGATHER,CURRENTSCATTER,N_TIMECAT};
	using type_time = double;
	using type_size = unsigned;
	using type_threadcount = short;
	using type_countmap = std::vector<type_size>;
	using type_timingmap = std::vector<type_time>;
	using type_timingvector = std::vector<type_timingmap>;
	using type_clock = util::rdtsc_timer;
	using type_clockvector = std::vector<type_clock>;
    using type_error = int;

	analysis()
		:	count_InitialNodes_(type_countmap()) // still necessary? 
		,	count_ProcessedNodes_(type_countmap()) // set in function
		,	count_ProcessedEdges_(type_countmap()) // set in function
		,	count_LastSyncVal_(type_countmap()) // still necessary?
		,	time_Total_(0)
		,	nThreads_(0) // set in function
		,	nProcs_(0) // set in function
		,	clocks_(N_TIMECAT)
		,	timings_() // set in function
	{

		nThreads_ = omp_get_max_threads();
		nProcs_ = omp_get_num_procs();
		assert(nThreads_>0 && nProcs_>0);

		// std::cout << "\nNT:" << nThreads_ << std::flush;
		// std::cout << "\nNP:" << nProcs_ << std::flush;
	
		// TODO: Probably count_InitialNodes_ and count_LastSyncVal_ can be removed
		count_ProcessedNodes_ = type_countmap(nThreads_);
		count_ProcessedEdges_ = type_countmap(nThreads_);
		timings_ = type_timingvector(N_TIMECAT,type_timingmap(nThreads_));
	}
	

	type_countmap count_InitialNodes_;		// counts how many nodes each thread has initially
	type_countmap count_ProcessedNodes_;	// counts how many nodes each thread has processed in total
	type_countmap count_ProcessedEdges_;	// counts how many nodes each thread has processed in total
	type_countmap count_LastSyncVal_;		// keeps track of the last sync value of each thread
	type_time time_Total_;
	type_clockvector clocks_;
	type_clock totalclock_;
	type_timingvector timings_;
    
    type_threadcount nThreads_;
    type_threadcount nProcs_;
    std::string algorithmName_;
    type_size nNodes_;
    type_size nEdges_;
    type_size depth_;
    std::vector<type_size> nChildrenQuantiles_;
    std::vector<type_size> frontSizes_;
    std::string graphName_;
    type_error errorCode_;
    
	// FUNCTIONS
	
	// TODO: this can probably be removed
	inline void initialnodes(type_threadcount tid, type_size nNodes) {
		assert(tid>=0 && tid<nThreads_);
		// count_InitialNodes_[tid] = nNodes;
	}
	
	inline void processednodes(type_threadcount tid, type_size nNodes) {
		assert(tid>=0 && tid<nThreads_);
		count_ProcessedNodes_[tid] = nNodes;
	}
	
	inline void incrementProcessedNodes(type_threadcount tid) { // TODO: check if this can be used instead of processednodes (performance??)
		assert(tid>=0 && tid<nThreads_);
		++count_ProcessedNodes_[tid];
	}
	
    inline void incrementProcessedEdges(type_threadcount tid, type_size nEdges) { // TODO: check if this can be used instead of processednodes (performance??)
		assert(tid>=0 && tid<nThreads_);
		count_ProcessedEdges_[tid] += nEdges;
	}

    inline void frontSizeHistogram(type_size frontSize) {
        frontSizes_.push_back(frontSize);
    }

	inline void starttotaltiming();
	
	inline void starttiming(timecat c) {
		clocks_[c].start();
	}
	

	inline void stoptotaltiming();
	
	inline void stoptiming(type_threadcount tid, timecat c) {
		// c stands for the index of the time category we are measuring
		// tid is the thread id
		assert(tid>=0 && tid<nThreads_);
		clocks_[c].stop(); // stop timing
		timings_[c][tid] += clocks_[c].sec(); // get time in seconds and add to total (for given thread)
	}

    bool xmlAnalysis(std::string relativeDir);
private:
    std::string suggestBaseFilename();

};

#else // declare empty inline functions - they will disappear

struct analysis {

	enum timecat {BARRIER,SOLUTIONPUSHBACK,REQUESTVALUEUPDATE,CURRENTGATHER,CURRENTSCATTER,N_TIMECAT};
	using type_time = double;
	using type_size = unsigned;
	using type_clock = util::rdtsc_timer;
	using type_threadcount = short;
    using type_error = int;

	type_time time_Total_;
	type_clock totalclock_;

    type_threadcount nThreads_;
    type_threadcount nProcs_;
    std::string algorithmName_;
    type_size nNodes_;
    type_size nEdges_;
    type_size depth_;
    std::string graphName_;
    type_error errorCode_;
    std::vector<type_size> nChildrenQuantiles_;
    std::vector<type_size> frontSizes_;

	analysis()
		:	time_Total_(0)
		,	nThreads_(0) // set in function
		,	nProcs_(0) // set in function
	{

		nThreads_ = omp_get_max_threads();
		nProcs_ = omp_get_num_procs();
		assert(nThreads_>0 && nProcs_>0);

		// std::cout << "\nNT:" << nThreads_ << std::flush;
		// std::cout << "\nNP:" << nProcs_ << std::flush;
	
	}

	inline void initialnodes(type_threadcount tid, type_size nNodes) {}
	inline void processednodes(type_threadcount tid, type_size nNodes) {}
	inline void incrementProcessedNodes(type_threadcount tid) {} // TODO: check if this can be used instead of processednodes (performance??)
    inline void incrementProcessedEdges(type_threadcount tid, type_size nEdges){}
    inline void frontSizeHistogram(type_size frontSize) {}
	inline void starttotaltiming();
	inline void starttiming(timecat c) {}
	inline void stoptotaltiming();
	inline void stoptiming(type_threadcount tid, timecat c) {}
	inline void threadcount(type_threadcount n) {}
    bool xmlAnalysis(std::string relativeDir);
private:
    std::string suggestBaseFilename();
};

#endif // ENABLE_ANALYSIS==0


inline void analysis::starttotaltiming() {
	totalclock_.start();
}

inline void analysis::stoptotaltiming() {
	totalclock_.stop();
	time_Total_ = totalclock_.sec();
}


#endif // ANALYSIS_HPP

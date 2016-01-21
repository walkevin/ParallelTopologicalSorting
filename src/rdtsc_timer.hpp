// Author:  Mario S. Könz <mskoenz@gmx.net>
// Date:    05.09.2015 20:22:53 CEST
// File:    rdtsc_timer.hpp

#ifndef UTIL_RDTSC_TIMER_HEADER
#define UTIL_RDTSC_TIMER_HEADER

#include <chrono>
#include <iostream>

namespace util {
    namespace detail {
        struct rdtsc_struct {
            double cycle() const {
                return (uint64_t(hi) << 32) + lo;
            }
            uint32_t hi;
            uint32_t lo;
        };
        inline double operator-(rdtsc_struct const & end, rdtsc_struct const & begin) {
            return end.cycle() - begin.cycle();
        }
    }// end namespace detail
    
    class rdtsc_timer {
        using chrono_clock = std::chrono::high_resolution_clock;
    public:
        //--------------------------- methods ----------------------------------
        void start() {
            chrono_start_ = chrono_clock::now();
            asm volatile ("cpuid" : : "a" (0) : "bx", "cx", "dx" ); // CPUID exec-barrier
            asm volatile ("rdtsc" : "=a" ((cyc_start_).lo), "=d"((cyc_start_).hi)); // RDTSC
        }
        void stop() {
            asm volatile ("rdtsc" : "=a" ((cyc_stop_).lo), "=d"((cyc_stop_).hi)); // RDTSC
            asm volatile ("cpuid" : : "a" (0) : "bx", "cx", "dx" ); // CPUID exec-barrier
            chrono_stop_ = chrono_clock::now();
        }
        //------------------------- const methods ------------------------------
        double cycles() const {
            return cyc_stop_ - cyc_start_;
        }
        double sec() const {
            using sec_dur = std::chrono::duration<double, std::ratio<1, 1>>;
            auto res = std::chrono::duration_cast<sec_dur>(chrono_stop_ - chrono_start_);
            return res.count();
        }
        double usec() const {
            using usec_dur = std::chrono::duration<double, std::ratio<1, 1000000>>;
            auto res = std::chrono::duration_cast<usec_dur>(chrono_stop_ - chrono_start_);
            return res.count();
        }
        double nsec() const {
            using usec_dur = std::chrono::duration<double, std::ratio<1, 1000000000>>;
            auto res = std::chrono::duration_cast<usec_dur>(chrono_stop_ - chrono_start_);
            return res.count();
        }
    private:
        detail::rdtsc_struct cyc_start_, cyc_stop_;
        chrono_clock::time_point chrono_start_, chrono_stop_;
    };
    
    //------------------------- stream operator --------------------------------
    inline std::ostream & operator<<(std::ostream & os, rdtsc_timer const & arg) {
        os << "cycles measured with rdtsc:        " << arg.cycles() << " cycles" << std::endl;
        os << "time measured with high res clock: " << arg.sec() << " seconds";
        return os;
    }
} // end namespace util

#endif // UTIL_RDTSC_TIMER_HEADER

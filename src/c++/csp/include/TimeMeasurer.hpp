#ifndef _TIMEMEASURER_HPP
#define _TIMEMEASURER_HPP
#include <iostream>
#include <chrono>
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::nanoseconds;

class TimeMeasurer {
    public:
    high_resolution_clock::time_point start_time_;
    high_resolution_clock::time_point end_time_;
    long long ns_count_ = 0;
    long long ms_count_ = 0;
    long long micros_count_ = 0;
    long long micros_count_step = 0;
    
    TimeMeasurer() {};

    void start() {
        start_time_ = high_resolution_clock::now();
    }

    //   void toc() {
    //     end_time_ = high_resolution_clock::now();
    //   }
    void reset() {
        ms_count_ = 0;
        micros_count_ = 0;
    }
    
    void resume() {
        start_time_ = high_resolution_clock::now();
    }
    void pause() {
        end_time_ = high_resolution_clock::now();
        // ns_count_ += time_ns();
        ms_count_ += time_ms(); // truncated problem, smaller than 1ms -> 0ms.
        micros_count_ += time_us();
    }

    long long total_time_ns() const {
        return ns_count_;
    }

    long long time_ms() const {
        return std::chrono::duration_cast<milliseconds>(end_time_ - start_time_).count();;
    }

    long long time_us() {
        micros_count_step = std::chrono::duration_cast<microseconds>(end_time_ - start_time_).count();
        return micros_count_step;
    }

    long long time_ns() const {
        return std::chrono::duration_cast<nanoseconds>(end_time_ - start_time_).count();
    }

    void print_ms(std::string msg) const {
        std::cout << msg << ": ";
        // std::cout << std::chrono::duration_cast<milliseconds>(end_time_ - start_time_).count() << " ms" << std::endl;
        // std::cout << ms_count_/1000 << " ms" << std::endl;
        std::cout << micros_count_/1000 << " ms" << std::endl;

    }

    //   void print_us(std::string msg) const {
    //     std::cout << msg << ": ";
    //     print_us();
    //   }
    //   void print_us() const {
    //     std::cout << std::chrono::duration_cast<microseconds>(end_time_ - start_time_).count() << " us" << std::endl;
    //   }

    //   void print_ns(std::string msg) const {
    //     std::cout << msg << ": ";
    //     print_ns();
    //   }
    //   void print_ns() const {
    //     std::cout << std::chrono::duration_cast<nanoseconds>(end_time_ - start_time_).count() << " ns" << std::endl;
    //   }

    private:
    TimeMeasurer(const TimeMeasurer &);
    TimeMeasurer &operator=(const TimeMeasurer &);

 
};
#endif //_TIMEMEASURER_HPP

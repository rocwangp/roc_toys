#include "timer.h"
#include "logging.h"

namespace rtoys
{
    namespace util
    {

        Timer::Timer(const std::chrono::steady_clock::time_point& t, std::function<void()> cb)
            : time_(t),
              interval_(0),
              cb_(cb),
              periodic_(false)
        { 
        }

        Timer::Timer(const std::chrono::steady_clock::time_point& t, const std::chrono::milliseconds& interval, std::function<void()> cb)
            : time_(t),
              interval_(interval),
              cb_(cb)
        { 
            if(interval > std::chrono::milliseconds(0))
                periodic_ = true;
            else
                periodic_ = false;
        }

        Timer::Timer(const Timer& timer)
            : time_(timer.time_),
              interval_(timer.interval_),
              cb_(timer.cb_),
              periodic_(timer.periodic_)
        {  }

        Timer::Timer(Timer&& timer)
            : time_(std::move(timer.time_)),
              interval_(std::move(timer.interval_)),
              cb_(std::move(timer.cb_)),
              periodic_(std::move(timer.periodic_))
        {  }

        Timer& Timer::operator=(const Timer& timer)
        {
            time_ = timer.time_;
            interval_ = timer.interval_;
            cb_ = timer.cb_;
            periodic_ = timer.periodic_;
            return *this;
        }

        void Timer::run() const
        {
            cb_();
        }

        bool Timer::periodic() const { return periodic_; }


        void Timer::update()
        {
            time_ += interval_;
        }
     
        int Timer::milliseconds() const
        {
            return static_cast<int>(
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                                time_ - std::chrono::steady_clock::now()
                            ).count()
                        );
        } 

        int Timer::seconds() const
        {
            return static_cast<int>(
                        std::chrono::duration_cast<std::chrono::seconds>(
                                time_ - std::chrono::steady_clock::now()
                            ).count()
                        );
        }

        const std::chrono::steady_clock::time_point& Timer::time() const 
        {
            return time_;
        }


        bool operator<(const Timer& lhs, const Timer& rhs) 
        {
            return lhs.time() < rhs.time();
        }

        bool operator<(const Timer& lhs, const std::chrono::steady_clock::time_point& rhs)
        {
            return lhs.time() < rhs;
        }

        bool operator<(const std::chrono::steady_clock::time_point& lhs, const Timer& rhs)
        {
            return lhs < rhs.time();
        }

        bool operator>(const Timer& lhs, const std::chrono::steady_clock::time_point& rhs)
        {
            return lhs.time() > rhs;
        }

        bool operator>(const std::chrono::steady_clock::time_point& lhs, const Timer& rhs)
        {
            return lhs > rhs.time();
        }
    }
}
 



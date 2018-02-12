#pragma once


#include <chrono>
#include <functional>
#include <boost/operators.hpp>


namespace rtoys
{
    namespace util
    {

        class Timer : public boost::less_than_comparable<Timer>
        {
            public:
                Timer(const std::chrono::steady_clock::time_point& t, 
                      std::function<void()> cb);
                Timer(const std::chrono::steady_clock::time_point& t, 
                      const std::chrono::milliseconds& interval, 
                      std::function<void()> cb);
                Timer(const Timer& timer);
                Timer(Timer&& timer);
                

                bool periodic() const;
                void run() const;
                void update();
             
                int milliseconds() const;
                int seconds() const;
              
                const std::chrono::steady_clock::time_point& time() const;


            private:
                std::chrono::steady_clock::time_point time_;
                std::chrono::milliseconds interval_;
                std::function<void()> cb_;
                bool periodic_;
        };

        bool operator<(const Timer& lhs, const Timer& rhs) ;

        bool operator<(const Timer& lhs, const std::chrono::steady_clock::time_point& rhs);

        bool operator<(const std::chrono::steady_clock::time_point& lhs, const Timer& rhs);

        bool operator>(const Timer& lhs, const std::chrono::steady_clock::time_point& rhs);

        bool operator>(const std::chrono::steady_clock::time_point& lhs, const Timer& rhs);

    }
}

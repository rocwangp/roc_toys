#pragma once

#include <sys/epoll.h>

#include <vector>
namespace rtoys
{
    namespace net 
    {
        class Channel;

        class Poller
        {
            public:
                enum Event
                {
                    READ_EVENT = EPOLLIN,
                    WRITE_EVENT = EPOLLOUT,
                    ERR_EVENT = EPOLLERR,
                    NONE_EVENT = 0
                };

                enum { MAX_EVENTS_NUMS = 1024 };
                Poller();

                void updateChannel(Channel* channel);
                void wait(std::vector<Channel*>& activeChannels);
            private:
                int epollfd_;
                int eventNums_;
                std::vector<struct epoll_event> events_;
                
        };
    }
}

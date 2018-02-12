#include "poller.h"
#include "../net/channel.h"
#include "../util/logging.h"

#include <vector>

namespace rtoys
{
    namespace net
    {
        Poller::Poller()
            : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
              eventNums_(0),
              events_(MAX_EVENTS_NUMS)

        {

        }


        void Poller::updateChannel(Channel* channel)
        {
            int epollOpt = EPOLL_CTL_ADD;
            int fd = channel->fd();
            if(channel->isAdded())
            {
                if(channel->isNoneEvent())
                {
                    epollOpt = EPOLL_CTL_DEL;
                    channel->setDeleted();
                    --eventNums_;
                }
            }
            else
            {
                if(!channel->isNoneEvent())
                {
                    epollOpt = EPOLL_CTL_ADD;
                    channel->setAdded();
                    ++eventNums_;
                }
            }
            struct epoll_event event;
            event.events = channel->events();
            event.data.ptr = channel;
            ::epoll_ctl(epollfd_, epollOpt, fd, &event);
        }

        void Poller::wait(std::vector<Channel*>& activeChannels, int timeout)
        {
            if(static_cast<int>(events_.size()) < std::max(1, eventNums_))
                events_.resize(std::max(1, eventNums_));
            int n = ::epoll_wait(epollfd_, &events_[0], events_.size(), timeout);
            activeChannels.clear();
            for(int i = 0; i < n; ++i)
            {
                activeChannels.emplace_back(static_cast<Channel*>(events_[i].data.ptr));
                activeChannels.back()->setActiveEvents(events_[i].events); 
            }
        }
    }
}

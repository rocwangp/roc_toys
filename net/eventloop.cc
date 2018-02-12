#include "eventloop.h"
#include "channel.h"
#include "../base/poller.h"

namespace rtoys
{
    namespace net
    {
        EventLoop::EventLoop()
            : poller_(std::make_unique<Poller>()),
              quit_(false),
              threadId_(std::this_thread::get_id()),
              watcher_(std::make_unique<util::PipeWatcher>()),
              watchChannel_(std::make_unique<Channel>(this, watcher_->readFd()))
        {
            watchChannel_->onRead(
                            [this]
                            {
                                this->watcher_->clear();
                            }
                        );
        }

        EventLoop::~EventLoop()
        {

        }

        void EventLoop::loop()
        {
            watchChannel_->enableRead();
            while(!quit_)
            {
                int timeout = timers_.empty() ? -1 : timers_.begin()->milliseconds();
                poller_->wait(activeChannels_, timeout);                
                for(auto& channel : activeChannels_)
                    channel->handleEvents();
                activeChannels_.clear();
                for(auto&& func : pendingFuncs_)
                    func();
                pendingFuncs_.clear();
                const auto now = std::chrono::steady_clock::now();
                for(auto it = timers_.begin(); it != timers_.end(); )
                {
                    if(*it > now)
                        break;
                    it->run();
                    if(it->periodic())
                    {
                        util::Timer timer(*it);
                        timer.update();
                        timers_.insert(std::move(timer));
                    }
                    timers_.erase(it++);
                }
            }
        }

        void EventLoop::quit()
        {
            quit_ = true;
            wakeup(); 
        }

        void EventLoop::wakeup()
        {
            watcher_->notify();
        }

        void EventLoop::updateChannel(Channel* channel)
        {
            poller_->updateChannel(channel);
        }

        void EventLoop::safeCall(pending_func_type func)
        {
            if(std::this_thread::get_id() == threadId_)
                func();
            else
                appendPendingFunc(func);
        }
        void EventLoop::appendPendingFunc(pending_func_type func)
        {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                pendingFuncs_.emplace_back(std::move(func));
            }
            wakeup();
        }

        void EventLoop::runAt(const util::Timer& timer)
        {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if(timers_.empty() || *timers_.begin() > timer)
                {
                    timers_.insert(timer);
                }
                else
                {
                    timers_.insert(timer);
                }
            }
            wakeup();
        }

        void EventLoop::cancel(const util::Timer& timer)
        {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                timers_.erase(timer);
            }
        }
    }
}

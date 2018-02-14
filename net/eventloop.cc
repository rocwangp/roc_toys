#include "eventloop.h"
#include "channel.h"
#include "../base/poller.h"
#include "../util/logging.h"

#include <chrono>

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
            watchChannel_->onRead( [this] { this->watcher_->clear(); });
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

                for(auto it = timers_.begin(); it != timers_.end(); )
                {
                    if(it->milliseconds() > 0) break;
                    it->run();
                    if(it->periodic())
                    {
                        util::Timer timer = *it;
                        timer.update();
                        timers_.insert(std::move(timer));
                    }
                    it = timers_.erase(it);
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

  

        void EventLoop::cancel(const util::Timer& timer)
        {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                timers_.erase(timer);
            }
        }

        void EventLoop::setTimer(util::Timer timer)
        {
            bool toWakeup = false;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if(timers_.empty() || *timers_.begin() > timer)
                {
                    timers_.insert(std::move(timer));
                    toWakeup = true;
                }
                else
                {
                    timers_.insert(std::move(timer));
                }
            }
            if(toWakeup)
                wakeup();
        }

        util::Timer EventLoop::runAt(const std::chrono::steady_clock::time_point& t, 
                                     std::function<void()> cb) 
        {
            return runAt(t, std::chrono::milliseconds(0), cb);
        }

        util::Timer EventLoop::runAt(const std::chrono::steady_clock::time_point& t, 
                                     const std::chrono::milliseconds& interval, 
                                     std::function<void()> cb)
        {
            return runEvery(t, interval, cb);
        }

        util::Timer EventLoop::runEvery(const std::chrono::milliseconds& interval, 
                                        std::function<void()> cb)
        {
            return runEvery(std::chrono::steady_clock::now() + interval, interval, cb);
        }

        util::Timer EventLoop::runEvery(const std::chrono::steady_clock::time_point& t, 
                                        const std::chrono::milliseconds& interval, 
                                        std::function<void()> cb)
        {
            util::Timer timer(t, interval, cb);
            setTimer(timer);
            return timer;
        }
        
        util::Timer EventLoop::runAfter(const std::chrono::milliseconds& interval, std::function<void()> cb)
        {
            return runAfter(interval, std::chrono::milliseconds(0), cb);
        }
        util::Timer EventLoop::runAfter(const std::chrono::milliseconds& t, 
                                        const std::chrono::milliseconds& interval, 
                                        std::function<void()> cb)
        {
            return runEvery(std::chrono::steady_clock::now() + t, interval, cb);
        }

    }
}

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
                poller_->wait(activeChannels_);                
                for(auto& channel : activeChannels_)
                    channel->handleEvents();
                activeChannels_.clear();
                for(auto&& func : pendingFuncs_)
                    func();
                pendingFuncs_.clear();
            }
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
            watcher_->notify(); 
        }
    }
}

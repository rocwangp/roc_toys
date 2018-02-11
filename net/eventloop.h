#pragma once

#include "../util/noncopyable.h"
#include "../util/watcher.h"
#include <memory>
#include <atomic>
#include <vector>
#include <thread>
#include <mutex>

namespace rtoys
{
    namespace net
    {
        class Poller;
        class Channel;

        class EventLoop : rtoys::util::noncopyable
        {
            public:
                typedef std::function<void()> pending_func_type;
            public:
                EventLoop();
                ~EventLoop();

                void loop();
                void updateChannel(Channel* channel);
                void safeCall(pending_func_type func);
                void appendPendingFunc(pending_func_type func);
                void wakeup();
            private:
                std::unique_ptr<Poller> poller_; 
                std::atomic<bool> quit_;
                std::vector<Channel*> activeChannels_;
                std::vector<pending_func_type> pendingFuncs_;
                std::thread::id threadId_;
                std::mutex mutex_;
                std::unique_ptr<rtoys::util::PipeWatcher> watcher_;
                std::unique_ptr<Channel> watchChannel_;
        };
    }
}

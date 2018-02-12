#pragma once

#include "../util/noncopyable.h"
#include "../util/watcher.h"
#include "../util/timer.h"

#include <memory>
#include <atomic>
#include <set>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <future>

namespace rtoys
{
    namespace net
    {
        class Poller;
        class Channel;

        class EventLoop : util::noncopyable
        {
            public:
                typedef std::function<void()> pending_func_type;
            public:
                EventLoop();
                ~EventLoop();

                void loop();
                void quit();
                void updateChannel(Channel* channel);
                void safeCall(pending_func_type func);
                void appendPendingFunc(pending_func_type func);
                void wakeup();
            public:
                void runAt(const util::Timer& timer);
                void cancel(const util::Timer& timer);

            private:
                std::unique_ptr<Poller> poller_; 
                std::atomic<bool> quit_;
                std::vector<Channel*> activeChannels_;
                std::vector<pending_func_type> pendingFuncs_;
                std::thread::id threadId_;
                std::mutex mutex_;
                std::unique_ptr<util::PipeWatcher> watcher_;
                std::unique_ptr<Channel> watchChannel_;
                std::set<util::Timer> timers_;
        };
    }
}

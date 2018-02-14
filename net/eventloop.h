#pragma once

#include "../util/noncopyable.h"
#include "../util/watcher.h"
#include "../util/timer.h"

#include <memory>
#include <atomic>
#include <queue>
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
                void cancel(const util::Timer& timer);

                util::Timer runAt(const std::chrono::steady_clock::time_point& t, std::function<void()> cb);
                util::Timer runAt(const std::chrono::steady_clock::time_point& t, const std::chrono::milliseconds& interval, std::function<void()> cb);

                util::Timer runEvery(const std::chrono::milliseconds& interval, std::function<void()> cb);
                util::Timer runEvery(const std::chrono::steady_clock::time_point& t, 
                              const std::chrono::milliseconds& interval, 
                              std::function<void()> cb);

                util::Timer runAfter(const std::chrono::milliseconds& interval, std::function<void()> cb);
                util::Timer runAfter(const std::chrono::milliseconds& t, const std::chrono::milliseconds& interval, std::function<void()> cb);

                void setTimer(util::Timer timer);

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
                /* std::priority_queue<util::Timer, std::vector<util::Timer>, std::greater<util::Timer>> timers_; */
        };
    }
}

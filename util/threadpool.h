#pragma once

#include <cstdint>
#include <thread>
#include <future>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

namespace rtoys
{
    namespace util
    {
        class ThreadPool
        {
            public:
                ThreadPool(std::size_t threadNums = 16);

                void start();
                void quit();
            public:
                template <class F, class... Args>
                auto enqueue(F&& f, Args... args)
                    -> std::future<typename std::result_of<F(Args...)>::type>;

            private:
                std::atomic<bool> quit_;
                std::size_t threadNums_;
                std::mutex mutex_;
                std::condition_variable cond_;
                std::vector<std::thread> threads_;
                std::queue<std::function<void()>> tasks_;
        };
    }
}

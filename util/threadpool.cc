#include "threadpool.h"

namespace rtoys
{
    namespace util
    {
        ThreadPool::ThreadPool(std::size_t threadNums)
            : quit_(false),
              threadNums_(threadNums)
        {

        }

        
        void ThreadPool::start()
        {
            for(std::size_t i = 0; i < threadNums_; ++i)
            {
                threads_.emplace_back(
                            [this]
                            {
                                while(!quit_)
                                {
                                    std::function<void()> task;
                                    {
                                        std::unique_lock<std::mutex> lock(mutex_);
                                        cond_.wait(lock, [this] { return quit_ || !tasks_.empty(); }); 
                                        if(quit_)   return;
                                        task = tasks_.front();
                                        tasks_.pop();
                                    }
                                    task();
                                }
                            }
                        );
            }
        }

        void ThreadPool::quit()
        {
            quit_ = true;
        }


        template <class F, class... Args>
        auto ThreadPool::enqueue(F&& f, Args... args)
            -> std::future<typename std::result_of<F(Args...)>::type>
        {
            using return_type = typename std::result_of<F(Args...)>::type;
            auto task = std::make_shared<std::packaged_task<return_type()>>(
                        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );
            std::future<return_type> result = task->get_future();
            {
                std::unique_lock<std::mutex> lock(mutex_);
                tasks_.emplace(
                            [task]
                            {
                                (*task)();
                            }
                        );
            }
            cond_.notify_one();
            return result;
        }
    }
}

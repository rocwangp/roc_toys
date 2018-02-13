#include "tcpserver.h"
#include "eventloop.h"
#include "acceptor.h"

#include "../util/logging.h"

namespace rtoys
{
    namespace net
    {
        TcpServer::TcpServer(const std::string& ip, unsigned short port, int threadNums)
            : TcpServer(new EventLoop(), ip, port, threadNums)
        {  }

        TcpServer::TcpServer(EventLoop* loop, const std::string& ip, unsigned short port, int threadNums)
            : loop_(std::shared_ptr<EventLoop>(loop)),
              acceptor_(std::make_unique<Acceptor>(loop_.get(), ip, port))
        {
            acceptor_->onAccept(
                            [this](int fd)
                            {
                                EventLoop* loop = nullptr;
                                if(loops_.empty())
                                    loop = this->loop_.get(); 
                                else
                                    loop = this->loops_[nextLoop_++];
                                nextLoop_ %= loops_.size();
                                
                                /* log_trace; */
                                auto conn = std::make_shared<Connection>(loop, fd);
                                conn->onBuild(this->connBuildCallBack_);
                                conn->onRead(this->connReadCallBack_);
                                conn->onWrite(this->connWriteCallBack_);
                                conn->onClose(
                                                [this]
                                                (std::shared_ptr<Connection> connPtr)
                                                {
                                                    this->loop_->safeCall(
                                                                [this, connPtr]
                                                                {
                                                                    this->connMap_.erase(connPtr->name());
                                                                }
                                                            );
                                                }
                                            );
                                auto name = conn->name();
                                connMap_.insert(std::make_pair(name, conn));
                                loop->safeCall(
                                                [conn]
                                                {
                                                    conn->connEstablished();
                                                }
                                            );
                                /* log_info << name; */
                            }
                        );
            {
                /* log_trace; */
                int avaThreadNums = std::thread::hardware_concurrency();
                threadNums = avaThreadNums == 0 ? threadNums : avaThreadNums;
                std::vector<std::promise<EventLoop*>> promises(threadNums);
                for(int i = 0; i < threadNums; ++i)
                {
                    threads_.emplace_back(
                                    [&loopPromise = promises[i]]
                                    {
                                        EventLoop loop;
                                        loopPromise.set_value(&loop);
                                        loop.loop();
                                    }
                                );
                }
                for(auto&& loopPromise : promises)
                    loops_.emplace_back(loopPromise.get_future().get());
                nextLoop_ = 0;
            }

            acceptor_->start();
        }

        TcpServer::~TcpServer()
        {

        }
        void TcpServer::start(int threadNums)
        {
            loop_->loop();
        }

        /* template <class F, class... Args> */
        /* auto TcpServer::onConnRead(F&& f, Args... args) */
        /*     -> std::future<typename std::result_of<F(Args...)>::type> */
        /* { */
        /*     using return_type = typename std::result_of<F(Args...)>::type; */
        /*     auto task = std::make_shared<std::packaged_task<return_type()>>( */
        /*                     std::bind(std::forward<F>(f), std::forward<Args>(args)...)); */
        /*     std::future<return_type> result = task->get_future(); */
        /*     connReadCallBack_ = */ 
        /*         [task](const std::shared_ptr<Connection>& conn) */
        /*         { */
        /*             (*task)(conn); */
        /*         }; */
        /*     return result; */
        /* } */

        
        /* util::Timer TcpServer::setTimer(const std::chrono::steady_clock::time_point& t, */
        /*                          const std::chrono::milliseconds& interval, */
        /*                          std::function<void()> cb) */
        /* { */
        /*     EventLoop* loop = loop_.get(); */
        /*     if(!loops_.empty()) */
        /*     { */
        /*         loop = loops_[nextLoop_++]; */
        /*         nextLoop_ %= loops_.size(); */
        /*     } */
        /*     util::Timer timer(t, interval, cb); */ 
        /*     loop->safeCall( */
        /*                     [loop, timer] */
        /*                     { */
        /*                         loop->setTimer(timer); */
        /*                     } */
        /*                 ); */
        /*     return timer; */
        /* } */

        /* util::Timer TcpServer::runAt(const std::chrono::steady_clock::time_point& t, std::function<void()> cb) */ 
        /* { */
        /*     return setTimer(t, std::chrono::milliseconds(0), cb); */ 
        /* } */

        /* util::Timer TcpServer::runEvery(const std::chrono::steady_clock::time_point& t, */ 
        /*                          const std::chrono::milliseconds& interval, */ 
        /*                          std::function<void()> cb) */
        /* { */
        /*    return setTimer(t, interval, cb); */ 
        /* } */
        
        /* util::Timer TcpServer::runAfter(const std::chrono::milliseconds& interval, std::function<void()> cb) */
        /* { */
        /*     return runAt(std::chrono::steady_clock::now() + interval, cb); */
        /* } */

        /* void TcpServer::cancel(const util::Timer& timer) */
        /* { */
        /* } */
    }
}

#include "tcpserver.h"
#include "eventloop.h"
#include "acceptor.h"

#include "../util/logging.h"

namespace rtoys
{
    namespace net
    {
        TcpServer::TcpServer(const std::string& ip, unsigned short port)
            : loop_(std::make_shared<EventLoop>()),
              acceptor_(std::make_unique<Acceptor>(loop_.get(), ip, port))
        {
            acceptor_->onAccept(
                            [this](int fd)
                            {
                                EventLoop* loop = nullptr;
                                if(nextLoop_ >= static_cast<int>(loops_.size()))
                                    loop = this->loop_.get(); 
                                else
                                    loop = this->loops_[nextLoop_++];
                                nextLoop_ %= loops_.size();
                                
                                auto conn = std::make_shared<Connection>(loop, fd);
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
                            }
                        );
        }

        TcpServer::~TcpServer()
        {

        }
        void TcpServer::start(int threadNums)
        {
            {
                log_trace;
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
    }
}

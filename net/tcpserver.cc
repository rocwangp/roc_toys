#include "tcpserver.h"
#include "eventloop.h"
#include "acceptor.h"

#include "../util/logging.h"

namespace rtoys
{
    namespace net
    {

        TcpServer::TcpServer(EventLoop* loop, const std::string& ip, unsigned short port, int threadNums)
            : loop_(loop),
              acceptor_(std::make_unique<Acceptor>(loop_, ip, port)),
              nextLoop_(-1)
        {
            acceptor_->onAccept( [this](int fd) { this->handleAccept(fd); });
            acceptor_->start();

            int avaThreadNums = std::thread::hardware_concurrency();
            threadNums = avaThreadNums == 0 ? threadNums : avaThreadNums;
            std::vector<std::promise<EventLoop*>> promises(threadNums);
            for(int i = 0; i < threadNums; ++i)
            {
                threads_.emplace_back(
                                [&loopPromise = promises[i], &ip, &port, this]
                                {
                                    EventLoop loop;
                                    loopPromise.set_value(&loop);
                                    Acceptor acceptor(&loop,ip, port);
                                    acceptor.onAccept( [this](int fd) { this->handleAccept(fd); });
                                    acceptor.start();
                                    loop.loop();
                                }
                            );
            }
            for(auto&& loopPromise : promises)
                loops_.emplace_back(loopPromise.get_future().get());
        }

        TcpServer::~TcpServer()
        {

        }
  
        void TcpServer::handleAccept(int fd)
        {
            EventLoop* loop = loops_.empty() ? loop_ : loops_[(++nextLoop_) %= loops_.size()];
            auto conn = std::make_shared<Connection>(loop, fd);
            conn->onBuild(connBuildCallBack_);
            conn->onRead(connReadCallBack_);
            conn->onWrite(connWriteCallBack_);
            conn->onClose(
                    [this] (std::shared_ptr<Connection> connPtr)
                    {
                        this->loop_->safeCall(
                                [this, connPtr]
                                {
                                    this->connMap_.erase(connPtr->name());
                                }
                          );
                    }
                );
            connMap_.insert(std::make_pair(conn->name(), conn));
            loop->safeCall( [conn] { conn->connEstablished(); });
        }
    }
}

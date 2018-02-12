#include "connection.h"
#include "channel.h"
#include "eventloop.h"
#include "buffer.h"
#include "../util/logging.h"

namespace rtoys
{
    namespace net
    {
        Connection::Connection()
            : loop_(nullptr),
              readBuffer_(std::make_shared<Buffer>()),
              writeBuffer_(std::make_shared<Buffer>())
        {

        }
        Connection::Connection(EventLoop* loop, int fd)
            : loop_(loop),
              channel_(std::make_unique<Channel>(loop, fd)),
              endpoint_(rtoys::ip::tcp::address::v4(), fd),
              name_(endpoint_.localAddrToString() + endpoint_.peerAddrToString()),
              readBuffer_(std::make_shared<Buffer>()),
              writeBuffer_(std::make_shared<Buffer>())
        {

        }


        Connection::~Connection()
        {

        }
        void Connection::connEstablished()
        {
            channel_->onRead(
                            [this]
                            {
                                int n = rtoys::ip::tcp::socket::read(channel_->fd(), this->readBuffer_);
                                if(n <= 0)
                                {
                                    this->channel_->disableAll();
                                    auto self(shared_from_this());
                                    if(this->connCloseCallBack_)
                                        this->connCloseCallBack_(self);
                                }
                                else 
                                {
                                    this->readBuffer_->appendBytes(n);
                                    auto self(shared_from_this());
                                    if(this->connReadCallBack_)
                                        this->connReadCallBack_(self);
                                }
                            }
                        );
            channel_->onWrite(
                            [this]
                            {
                                log_trace;
                                auto self(shared_from_this());
                                if(this->connWriteCallBack_)
                                    this->connWriteCallBack_(self);
                                int n = rtoys::ip::tcp::socket::write(channel_->fd(), this->writeBuffer_);
                                if(n == this->writeBuffer_->size())
                                    channel_->disableWrite();
                                this->writeBuffer_->appendBytes(n);
                            }
                        );
            channel_->onClose(
                            [this]
                            {
                                log_trace;
                                this->channel_->disableAll();
                                auto self(shared_from_this());
                                if(this->connCloseCallBack_)
                                    this->connCloseCallBack_(self);
                            }
                        );
            if(connBuildCallBack_)
                connBuildCallBack_(shared_from_this());
            channel_->enableRead();
        }

        void Connection::send(const std::string& msg)
        {
            if(channel_->isWriteEvent())
            {
                writeBuffer_->append(msg);
            }
            else
            {
                log_debug << msg;
                int n = rtoys::ip::tcp::socket::write(channel_->fd(), msg);
                log_debug << n;
                if(n < static_cast<int>(msg.size()))
                {
                    n = std::max(n, 0);
                    writeBuffer_->append(msg.substr(n));
                    channel_->enableWrite();
                }
            }
        }

        std::string Connection::readAll()
        {
            return readBuffer_->retrieveAll();
        }
              
        std::shared_ptr<Buffer> Connection::readBuffer()
        {
            return readBuffer_;
        }

        void Connection::connect(const std::string& ip, unsigned short port)
        {
            static int count = 1;
            EventLoop loop;
            int fd = rtoys::ip::tcp::socket::nonblock_socket();
            ::rtoys::ip::tcp::socket::connect(fd, ip, port);
            channel_.reset(new Channel(&loop, fd));
            endpoint_.reset(rtoys::ip::tcp::address::v4(), fd);
            name_ = endpoint_.localAddrToString() + endpoint_.peerAddrToString();
            connEstablished();
            channel_->onClose(
                        [this, &ip, &port]
                        {
                            this->channel_->disableAll();
                            this->loop_->runAt(util::Timer(
                                                  std::chrono::steady_clock::now() + std::chrono::seconds(count),
                                                  [this, &ip, &port]
                                                  {
                                                    this->connect(ip, port);
                                                  }
                                            )
                                    );
                            count *= 2;
                        }
                    );
            loop.loop();
        }
    }
}

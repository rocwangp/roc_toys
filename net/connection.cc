#include "connection.h"
#include "channel.h"
#include "eventloop.h"
#include "buffer.h"
#include "../util/logging.h"

namespace rtoys
{
    namespace net
    {
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
                                log_debug << n;
                                log_debug << std::string(readBuffer_->begin(), readBuffer_->end());
                                if(n <= 0)
                                {
                                    this->channel_->disableAll();
                                    auto self(shared_from_this());
                                    this->connCloseCallBack_(self);
                                }
                                else 
                                {
                                    this->readBuffer_->appendBytes(n);
                                    auto self(shared_from_this());
                                    this->connReadCallBack_(self);
                                }
                            }
                        );
            channel_->onWrite(
                            [this]
                            {
                                log_trace;
                                auto self(shared_from_this());
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
                                this->connCloseCallBack_(self);
                            }
                        );
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
              
    }
}

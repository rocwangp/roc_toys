#include "connection.h"
#include "channel.h"
#include "eventloop.h"
#include "buffer.h"
#include "../util/logging.h"

namespace rtoys
{
    namespace net
    {
        Connection::Connection(EventLoop* loop)
            : loop_(loop),
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
                                int n = rtoys::ip::tcp::socket::read(channel_->fd(), readBuffer_);
                                if(n <= 0)
                                    close(); 
                                else 
                                {
                                    readBuffer_->appendBytes(n);
                                    auto self(shared_from_this());
                                    if(readcb_)
                                        readcb_(self);
                                }
                            }
                        );
            channel_->onWrite(
                            [this]
                            {
                                auto self(shared_from_this());
                                if(writecb_)
                                    writecb_(self);
                                int n = rtoys::ip::tcp::socket::write(channel_->fd(), writeBuffer_);
                                if(n == writeBuffer_->size())
                                    channel_->disableWrite();
                                this->writeBuffer_->appendBytes(n);
                            }
                        );
            channel_->onClose(
                            [this]
                            {
                                close(); 
                            }
                        );
            if(buildcb_)
                buildcb_(shared_from_this());
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
                int n = rtoys::ip::tcp::socket::write(channel_->fd(), msg);
                log_info(n);
                if(n < static_cast<int>(msg.size()))
                {
                    n = std::max(n, 0);
                    writeBuffer_->append(msg.substr(n));
                    channel_->enableWrite();
                }
            }
        }

        void Connection::close()
        {
            channel_->disableAll();
            if(closecb_)
                closecb_(shared_from_this());
        }

        std::string Connection::readAll()
        {
            return readBuffer_->retrieveAll();
        }

        std::string Connection::readUtil(const std::string& boundary)
        {
            return readBuffer_->readUtil(boundary);
        }
              

        void Connection::connect(const std::string& ip, unsigned short port)
        {
            int fd = rtoys::ip::tcp::socket::create_socket(rtoys::ip::tcp::socket::BLOCK);
            if(!::rtoys::ip::tcp::socket::connect(fd, ip, port))
            {
                log_error(ip, " ", port);
                throw std::runtime_error("connect error");
            }
            rtoys::ip::tcp::socket::set_nonblock(fd); 
            channel_.reset(new Channel(loop_, fd));
            endpoint_.reset(rtoys::ip::tcp::address::v4(), fd),
            name_ = (endpoint_.localAddrToString() + endpoint_.peerAddrToString()),
            connEstablished();
        }
    }
}

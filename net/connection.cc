#include "connection.h"
#include "channel.h"
#include "eventloop.h"
#include "buffer.h"
#include "../util/logging.h"

namespace rtoys
{
    namespace net
    {
        using namespace rtoys::ip::tcp;

        Connection::Connection(EventLoop* loop)
            : loop_(loop),
              readBuffer_(std::make_shared<Buffer>()),
              writeBuffer_(std::make_shared<Buffer>()),
              connInterval_(std::chrono::milliseconds(1000))
        {

        }

        Connection::Connection(EventLoop* loop, int fd)
            : loop_(loop),
              channel_(std::make_unique<Channel>(loop, fd)),
              endpoint_(rtoys::ip::tcp::address::v4(), fd),
              name_(endpoint_.localAddrToString() + endpoint_.peerAddrToString()),
              readBuffer_(std::make_shared<Buffer>()),
              writeBuffer_(std::make_shared<Buffer>()),
              connInterval_(std::chrono::milliseconds(1000))
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
                                int n = socket::read(channel_->fd(), readBuffer_);
                                if(n <= 0)
                                    close(); 
                                else 
                                {
                                    readBuffer_->appendBytes(n);
                                    if(readcb_)
                                        readcb_(shared_from_this());
                                }
                            }
                        );
            channel_->onWrite(
                            [this]
                            {
                                if(writecb_)
                                    writecb_(shared_from_this());
                                int n = socket::write(channel_->fd(), writeBuffer_);
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
            channel_->enableRead();
            if(buildcb_)
                buildcb_(shared_from_this());
        }

        void Connection::send(const std::string& msg)
        {
            if(channel_->isWriteEvent())
            {
                writeBuffer_->append(msg);
            }
            else
            {
                int n = socket::write(channel_->fd(), msg);
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
            int fd = socket::block_socket();
            if(!socket::connect(fd, ip, port))
            {
                log_error(ip, port, std::strerror(errno));
                ::close(fd);
                loop_->runAfter(connInterval_,
                                [this, ip, port]
                                {
                                    log_info("reconnct");
                                    connect(ip, port);
                                }
                            );
                return;
            }
            channel_.reset(new Channel(loop_, fd));
            socket::set_nonblock(fd); 
            endpoint_.reset(address::v4(), fd),
            name_ = (endpoint_.localAddrToString() + endpoint_.peerAddrToString()),
            connEstablished();
        }
    }
}

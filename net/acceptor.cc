#include "acceptor.h"
#include "channel.h"
#include "eventloop.h"

#include "../ip/sockets.h"
#include "../util/io.h"
#include "../util/logging.h"

namespace rtoys
{
    namespace net
    {
        using namespace rtoys::ip::tcp;

        Acceptor::Acceptor(EventLoop* loop, const std::string& ip, unsigned short port)
            : loop_(loop),
              channel_(std::make_unique<Channel>(loop)),
              idleFd_(util::io::open("/dev/null"))
        {
            if(!rtoys::ip::tcp::socket::bind(channel_->fd(), std::make_shared<address::v4>(ip, port)))
            {
                log_error(ip, port);
                ::perror("bind error)");
                throw std::runtime_error("bind error ");
            }
            if(!rtoys::ip::tcp::socket::listen(channel_->fd()))
            {
                log_error(ip, port);
                ::perror("listen error)");
                throw std::runtime_error("listen error");
            }
        }

        Acceptor::~Acceptor()
        {
            channel_->disableAll();
            util::io::close(idleFd_);    
        }

        void Acceptor::start()
        {
             channel_->onRead(
                            [this]
                            {
                                int fd = socket::accept(channel_->fd());
                                if(fd == -1)
                                {
                                    if(errno == EMFILE)
                                    {
                                        log_error("file descriptor use out...");
                                        util::io::close(idleFd_);
                                        socket::close(socket::accept(channel_->fd()));
                                        idleFd_ = util::io::open("/dev/null");
                                    }
                                    return;
                                }
                                if(acceptCallBack_)
                                    acceptCallBack_(fd);
                            }
                        );
            channel_->enableRead();
        }
    }
}

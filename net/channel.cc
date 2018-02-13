#include "channel.h"
#include "eventloop.h"
#include "../base/poller.h"
#include "../ip/sockets.h"
#include "../util/logging.h"

namespace rtoys
{
    namespace net
    {
        Channel::Channel(EventLoop* loop)
            : Channel(loop, rtoys::ip::tcp::socket::nonblock_socket())
        {  }
        Channel::Channel(EventLoop* loop, int fd)
            : loop_(loop),
              fd_(fd),
              events_(Poller::NONE_EVENT),
              activeEvents_(0),
              state_(State::INIT)
        {
            
        }

        Channel::~Channel()
        {
            rtoys::ip::tcp::socket::close(fd_);
        }

        int Channel::fd()
        {
            return fd_;
        }

        bool Channel::readable()
        {
            return activeEvents_ & (Poller::READ_EVENT | Poller::ERR_EVENT);
        }
        bool Channel::writeable()
        {
            return activeEvents_ & (Poller::WRITE_EVENT);
        }
        bool Channel::none()
        {
            return !(readable() || writeable());
        }
        bool Channel::isReadEvent()
        {
            return events_ & rtoys::net::Poller::READ_EVENT;
        }
        bool Channel::isWriteEvent()
        {
            return events_ & rtoys::net::Poller::WRITE_EVENT;
        }
        bool Channel::isNoneEvent()
        {
            return !(isReadEvent() || isWriteEvent());
        }

        void Channel::enableRead()
        {
            events_ |= Poller::READ_EVENT;
            update();
        }
        void Channel::enableWrite()
        {
            events_ |= Poller::WRITE_EVENT;
            update();
        }
        void Channel::disableRead()
        {
            events_ &= ~(Poller::READ_EVENT);
            update();
        }
        void Channel::disableWrite()
        {
            events_ &= ~(Poller::WRITE_EVENT);
            update();
        }
        void Channel::disableAll()
        {
            events_ = Poller::NONE_EVENT;
            update();
        }

        void Channel::update()
        {
            loop_->updateChannel(this);
        }

        void Channel::handleEvents()
        {
            if(readable())
            {
                readCallBack_();
            }
            else if(writeable())
            {
                writeCallBack_();
            }
            else
            {
                /* error */
                /* log_error << "unknown channel events"; */
                disableAll();
            }
        }
    }
}

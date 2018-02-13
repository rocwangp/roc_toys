#pragma once

#include <memory>
#include <functional>

#include "../util/noncopyable.h"

namespace rtoys
{
    namespace net
    {
        class EventLoop;
        class Channel;
        
        class Acceptor : private util::noncopyable
        {
            public:
                typedef std::function<void(int)> accept_cb_type;
            public:
                Acceptor(EventLoop* loop, const std::string& ip, unsigned short port);
                ~Acceptor();

                void start();
                
                void onAccept(accept_cb_type cb) { acceptCallBack_ = cb; }
            private:
                EventLoop* loop_;
                std::unique_ptr<Channel> channel_;
                int idleFd_;

                accept_cb_type acceptCallBack_;
        };
    }
}

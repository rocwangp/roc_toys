#pragma once

#include "../util/noncopyable.h"
#include "../ip/sockets.h"
#include "buffer.h"

#include <memory>
#include <string>


namespace rtoys
{
    namespace net
    {
        class EventLoop;
        class Channel;

        class Connection : 
            public std::enable_shared_from_this<Connection>,
            private util::noncopyable
        {
            public:
                typedef std::function<void(const std::shared_ptr<Connection>&)> conn_cb_type;;
            public:
                Connection(EventLoop* loop);
                Connection(EventLoop* loop, int fd);
                ~Connection(); 

                void connEstablished();
                void connDestroyed();
                std::string name() const { return name_; }
                
                void onBuild(conn_cb_type cb) { buildcb_ = cb; }
                void onRead(conn_cb_type cb) { readcb_ = cb; }
                void onWrite(conn_cb_type cb) { writecb_ = cb; }
                void onClose(conn_cb_type cb) { closecb_ = cb; }

                void connect(const std::string& ip, unsigned short port);
                void send(const std::string& msg);
                void close();

                std::string readAll();
                std::string readUtil(const std::string& boundary);

            private:
                EventLoop *loop_;
                std::unique_ptr<Channel> channel_;
                ip::tcp::endpoint endpoint_;
                std::string name_;
                std::shared_ptr<Buffer> readBuffer_;
                std::shared_ptr<Buffer> writeBuffer_;
                conn_cb_type buildcb_, readcb_, writecb_, closecb_;
        };
    }
}

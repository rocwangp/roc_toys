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
                typedef std::function<void(const std::shared_ptr<Connection>&)> conn_read_type;
                typedef std::function<void(const std::shared_ptr<Connection>&)> conn_write_type;
                typedef std::function<void(const std::shared_ptr<Connection>&)> conn_close_type;
            public:
                Connection(EventLoop* loop, int fd);
                ~Connection(); 

                void connEstablished();
                void connDestroyed();
                std::string name() const { return name_; }
                
                void onRead(conn_read_type cb) { connReadCallBack_ = cb; }
                void onWrite(conn_write_type cb) { connWriteCallBack_ = cb; }
                void onClose(conn_close_type cb) { connCloseCallBack_ = cb; }

                void send(const std::string& msg);

                std::string readAll();
            private:
                EventLoop *loop_;
                std::unique_ptr<Channel> channel_;
                ip::tcp::endpoint endpoint_;
                std::string name_;
                conn_read_type connReadCallBack_;
                conn_write_type connWriteCallBack_;
                conn_close_type connCloseCallBack_;

                std::shared_ptr<Buffer> readBuffer_;
                std::shared_ptr<Buffer> writeBuffer_;
        };
    }
}

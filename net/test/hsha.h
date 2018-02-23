#pragma once

#include <memory>
#include <string>

#include "tcpserver.h"
#include "../util/threadpool.h"

namespace rtoys
{
    namespace net
    {
        class HSHA
        {
            public:
                typedef std::function<void(const std::shared_ptr<Connection>&)> conn_cb_type;
            public:
                HSHA(EventLoop* loop, const std::string& ip, unsigned short port);
                ~HSHA();

            public:
                void onRead(conn_cb_type cb); 
            private:
                TcpServer server_;
                util::ThreadPool pool_;
                conn_cb_type readcb_;
        };
    }
}

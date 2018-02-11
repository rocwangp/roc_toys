#pragma once

#include "../ip/sockets.h"

namespace rtoys
{
    namespace net
    {
        class Socket
        {
            public:
                explicit Socket(int fd = rtoys::ip::tcp::socket::nonblock_socket())
                    : fd_(fd)
                {

                }

                ~Socket()
                {
                    rtoys::ip::tcp::socket::close(fd_);
                }
            public:
                int fd()
                {
                    return fd_;
                }
            private:
                int fd_;
        };



    }
}


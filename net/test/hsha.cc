#include "hsha.h"
#include "tcpserver.h"

#include "../util/threadpool.h"

namespace rtoys
{
    namespace net
    {
        HSHA::HSHA(EventLoop* loop, const std::string& ip, unsigned short port)
            : server_(loop, ip, port, 0)
        {
            pool_.start();
        }

        HSHA::~HSHA()
        {
            pool_.quit();
        }

        void HSHA::onRead(conn_cb_type cb)
        {
            server_.onConnRead(
                        [this, cb](const auto& conn)
                        {
                            pool_.enqueue(cb, conn);
                        }
                    );
        }
    }
}

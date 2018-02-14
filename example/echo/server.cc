#include "../../rtoys.h"

int main()
{
    rtoys::net::EventLoop base;
    rtoys::net::TcpServer server(&base, "localhost", 9999);
    server.onConnRead(
                    [](const auto& conn)
                    {
                        conn->send(conn->readAll());
                    }
                );
    base.loop();
    return 0;
}

#include "../../net/tcpserver.h"
int main()
{
    rtoys::net::TcpServer server("localhost", 9999);
    server.onConnRead(
                    [](const auto& conn)
                    {
                        conn->send(conn->readAll());
                    }
                );
    server.start();
    return 0;
}

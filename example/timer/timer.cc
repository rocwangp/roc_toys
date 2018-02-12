#include "../../net/tcpserver.h"
#include <iostream>
int main()
{
    rtoys::net::TcpServer server("localhost", 9999);
    auto now = std::chrono::steady_clock::now();
    server.runAt(now + std::chrono::seconds(10),
                [&now]
                {
                    log_info("once timer run...");
                    log_info << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - now).count(); 
                }
            );
    auto timer = server.runEvery(now + std::chrono::seconds(5),
                    std::chrono::seconds(5),
                    [&now]
                    {
                        log_info("periodic timer run...");
                        log_info <<  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - now).count();
                    }
                );
    server.runAfter(std::chrono::seconds(7),
                    [&server, &now, &timer]
                    {
                        server.cancel(timer); 
                        log_info("after timer run...");
                        log_info <<  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - now).count();
                    }
                );
    server.start();
    return 0;
}

#include "../../net/connection.h"
#include "../../net/eventloop.h"
#include <thread>
#include <string>
#include <iostream>

int main()
{
    rtoys::net::EventLoop base;
    auto conn = std::make_shared<rtoys::net::Connection>(&base);
    conn->onRead(
                [](const auto& connPtr)
                {
                    log_info(connPtr->readAll());
                }
            );
    std::thread t(
                [conn]
                {
                    std::string line;
                    while(std::getline(std::cin, line))
                    {
                        conn->send(line);
                    }
                }
            );
    conn->connect("localhost", 9999);
    base.loop();
    t.join();
    return 0;
}

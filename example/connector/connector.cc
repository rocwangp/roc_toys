#include "../../rtoys.h"
#include <iostream>

int main()
{
    rtoys::net::EventLoop base;
    auto conn = std::make_shared<rtoys::net::Connection>(&base);
    conn->setConnInterval(std::chrono::milliseconds(1000));
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
                        if(conn->isConnected())
                            conn->send(line);
                        else
                            log_error("no connection");
                    }
                }
            );
    conn->connect("localhost", 9999);
    base.loop();
    t.join();
    return 0;
}

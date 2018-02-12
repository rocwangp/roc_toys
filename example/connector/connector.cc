#include "../../net/connection.h"
#include <thread>
#include <string>
#include <iostream>

int main()
{
    auto conn = std::make_shared<rtoys::net::Connection>();
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
    t.join();
    return 0;
}

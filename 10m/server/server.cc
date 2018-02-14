#include "../../net/tcpserver.h"
#include "../../net/eventloop.h"
#include "../../util/slice.h"

#include <iostream>
#include <string>
#include <unistd.h>

#include <vector>
#include <unordered_map>

using namespace rtoys::net;


struct Report
{
    int pid;
    long int connected;
    long int closed;
    long int recved;
};

int main(int argc, char* argv[])
{
    if(argc < 5)
    {
        std::cout << "usage: " << argv[0] << " <begin port> <end port> <processes> <server managed port> <client managed port>" << std::endl;
        return 1;
    }
    int beginPort = std::strtol(argv[1], nullptr, 10);
    int endPort = std::strtol(argv[2], nullptr, 10);
    int processes = std::strtol(argv[3], nullptr, 10);
    int managePort = std::strtol(argv[4], nullptr, 10);
    int clientManPort = std::strtol(argv[5], nullptr, 10);
    int pid = 1;
    for(int i = 0; i < processes; ++i)
    {
        pid = ::fork();
        if(pid == 0)
            break;
    }

    log_info(beginPort, " ", endPort, " ", processes, " ", managePort, " ", clientManPort);
    EventLoop loop;
    if(pid == 0)
    {
        ::sleep(1);
        std::vector<std::shared_ptr<TcpServer>> servers;
        long connected = 0, closed = 0, recved = 0;
        for(int port = beginPort; port <= endPort; ++port)
        {
            auto server = std::make_shared<TcpServer>(&loop, "localhost", port);
            server->onConnBuild(
                        [&](const auto&)
                        {
                            ++connected; 
                        }
                    );
            server->onConnRead(
                        [&](const auto& conn)
                        {
                            ++recved;
                            conn->send(conn->readAll());
                        }
                    );
            server->onConnClose(
                        [&](const auto&)
                        {
                            log_info("close");
                            ++closed;
                        }
                    );
            servers.push_back(server);
        }
        std::shared_ptr<Connection> report;
        loop.runAfter(std::chrono::milliseconds(2000),
                      [&]
                      {
                        log_info("try to connect to client ", clientManPort);
                        report = std::make_shared<Connection>(&loop);
                        report->connect("127.0.0.1", clientManPort);
                        log_info("connect to client");
                      });
        loop.runAfter(std::chrono::milliseconds(3000),
                      std::chrono::milliseconds(2000),
                      [&]
                      {
                          std::string msg(rtoys::util::format("pid %d connected %ld closed %ld recved %ld", ::getpid(), connected, closed, recved));
                          log_info(msg);
                          report->send(msg);
                      });
        
        log_info("done");
        loop.loop();
    }
    else
    {
        std::unordered_map<int, Report> reports;
        auto master = std::make_shared<TcpServer>(&loop, "127.0.0.1", managePort);
        master->onConnBuild(
                    [&](const auto&)
                    {
                        log_info("client connection");
                    }
                );
        master->onConnRead(
                    [&](const auto& conn)
                    {
                        log_info("connRead");
                        rtoys::util::Slice slice(conn->readAll(), ' '); 
                        Report& report = reports[std::strtol(slice[1].c_str(), nullptr, 10)];
                        report.pid = std::strtol(slice[1].c_str(), nullptr, 10);
                        report.connected = std::strtoll(slice[3].c_str(), nullptr, 10);
                        report.closed = std::strtoll(slice[5].c_str(), nullptr, 10);
                        report.recved = std::strtoll(slice[7].c_str(), nullptr, 10);
                    }
                );
        loop.runEvery(std::chrono::steady_clock::now(),
                         std::chrono::milliseconds(3000),
                        [&]
                        {
                            log_trace;
                            for(auto& p : reports)
                            {
                                ::printf("pid %6d connected %6ld closed %6ld recved %6ld\n", 
                                        p.second.pid, p.second.connected, p.second.closed, p.second.recved);    
                            }
                        });
        loop.loop();
    }
}

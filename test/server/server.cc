#include "../../rtoys.h"

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

int toInt(const char* str)
{
    return std::strtol(str, nullptr, 10);
}

int main(int argc, char* argv[])
{
    if(argc < 5)
    {
        std::cout << "usage: " << argv[0] << " <begin port> <end port> <processes> <server managed port> <client managed port>" << std::endl;
        return 1;
    }
    int beginPort = toInt(argv[1]);
    int endPort = toInt(argv[2]);
    int processes = toInt(argv[3]);
    int managePort = toInt(argv[4]);
    int clientManPort = toInt(argv[5]);
    log_info(beginPort,  endPort,  processes,  managePort,  clientManPort);
    int pid = 1;
    for(int i = 0; i < processes; ++i)
    {
        pid = ::fork();
        if(pid == 0)
            break;
    }

    EventLoop loop;
    if(pid == 0)
    {
        int build = 0;
        std::vector<std::shared_ptr<TcpServer>> servers;
        long connected = 0, closed = 0, recved = 0;
        for(int port = beginPort; port <= endPort; ++port)
        {
            auto server = std::make_shared<TcpServer>(&loop, "127.0.0.1", port);
            server->onConnBuild(
                        [&, port](const auto&)
                        {
                            log_info(++build, port);
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
                            ++closed;
                        }
                    );
            servers.push_back(server);
            log_info(port);
        }
        /* Connection report(&loop); */
        /* report.setConnInterval(std::chrono::milliseconds(1000)); */
        /* log_info("done"); */
        /* report.connect("127.0.0.1", clientManPort); */

        /* loop.runAfter(std::chrono::milliseconds(2000), std::chrono::milliseconds(2000), */
        /*               [&] */
        /*               { */
        /*                   std::string msg(rtoys::util::format("pid %d connected %ld closed %ld recved %ld", */ 
        /*                                   ::getpid(), connected, closed, recved)); */
        /*                   report.send(msg); */
        /*               }); */
        
        log_info("done");
        loop.loop();
    }
    else
    {
        std::unordered_map<int, Report> reports;
        TcpServer master(&loop, "127.0.0.1", managePort);
        master.onConnBuild( [&](const auto&) { log_info("client connection"); });
        master.onConnRead(
                    [&](const auto& conn)
                    {
                        /* log_info("connRead"); */
                        rtoys::util::Slice slice(conn->readAll(), ' '); 
                        Report& report = reports[std::strtol(slice[1].c_str(), nullptr, 10)];
                        report.pid = std::strtol(slice[1].c_str(), nullptr, 10);
                        report.connected = std::strtoll(slice[3].c_str(), nullptr, 10);
                        report.closed = std::strtoll(slice[5].c_str(), nullptr, 10);
                        report.recved = std::strtoll(slice[7].c_str(), nullptr, 10);
                    }
                );
        loop.runEvery(std::chrono::milliseconds(3000),
                        [&]
                        {
                            for(auto& p : reports)
                            {
                                ::printf("pid %6d connected %6ld closed %6ld recved %6ld\n", 
                                        p.second.pid, p.second.connected, p.second.closed, p.second.recved);    
                            }
                        });
        loop.loop();
    }
}

#include "../../rtoys.h"

#include <vector>
#include <string>
#include <iostream>

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
    if(argc < 9)
    {
        ::printf("usage %s <host> <begin port> <end port> <conn count> <create seconds> <processes> <heartbeat interval> <send size> <management port> <server port>\n",argv[0]);
        return 1;
    }
    int c = 1;
    std::string host(argv[c++]);
    int beginPort = std::atoi(argv[c++]);
    int endPort = std::atoi(argv[c++]);
    int connCnt = std::atoi(argv[c++]);
    float createSec = std::atof(argv[c++]);
    int processes = std::atoi(argv[c++]);
    connCnt /= processes;
    int hearbeatInterval = std::atoi(argv[c++]);
    int bsz = std::atoi(argv[c++]);
    int managePort = std::atoi(argv[c++]);
    int serverManPort = std::atoi(argv[c++]);

    log_info(host,  beginPort, endPort,  connCnt, createSec, processes, managePort, serverManPort);
    int pid = 1;
    for(int i = 0; i < processes; ++i)
    {
        pid = ::fork();
        if(pid == 0)
        {
            ::sleep(1);
            break;
        }
    }

    EventLoop loop;
    if(pid == 0)
    {
        int send = 0;
        int recv = 0;
        int connected = 0;
        int closed = 0;

        int build = 0;
        std::vector<std::shared_ptr<Connection>> conns;
        for(int k = 0; k < static_cast<int>(createSec * 10); ++k)
        {
            loop.runAfter(std::chrono::milliseconds(k * 100),
                         [&, k]
                         {
                            log_info(k);
                            int c = connCnt / createSec / 10;
                            for(int i = 0; i < c; ++i)
                            {
                                short int port = beginPort + (i % (endPort - beginPort));
                                auto conn = std::make_shared<Connection>(&loop);
                                conn->setConnInterval(std::chrono::milliseconds(500));
                                conn->onBuild(
                                            [&](const auto&)
                                            {
                                                log_info(++build, port);
                                                ++connected;
                                            }
                                        );
                                conn->onRead(
                                            [&](const auto& connPtr)
                                            {
                                                ++recv;
                                                connPtr->send(connPtr->readAll());
                                                ++send;
                                            }
                                        );
                                conn->onClose(
                                            [&](const auto&)
                                            {
                                                --connected;
                                                ++closed;
                                            }
                                        );
                                conn->connect(host, port);
                                conns.push_back(conn);
                            }
                         });
        }

        /* log_info("done"); */
        /* Connection report(&loop); */
        /* report.connect("127.0.0.1", serverManPort); */
        /* loop.runEvery(std::chrono::milliseconds(3000), */
        /*                 [&] */
        /*                 { */
        /*                     std::string msg(rtoys::util::format("pid %d connected %ld send %ld recved %ld", */ 
        /*                                     ::getpid(), connected, send, recv)); */
        /*                     report.send(msg); */
                       
        /*                 }); */
        /* log_info("done"); */
        loop.loop();
    }
    else
    {
        std::unordered_map<int, Report> reports;
        TcpServer master(&loop, "127.0.0.1", managePort);
        master.onConnBuild( [&](const auto&) { log_info("server connection"); });
        master.onConnRead(
                    [&](const auto& conn)
                    {
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
     return 0;
}

#include "../../net/tcpserver.h"
#include "../../net/eventloop.h"
#include <iostream>
int main()
{
    int pid = 1;
    for(int i = 0; i < 10; ++i)
    {
        pid = ::fork();
        if(pid == 0)
            break;;
    }
    rtoys::net::EventLoop base;
    if(pid == 0)
    {

    for(int i = 0; i < 100; ++i)
    {
        base.runAfter(std::chrono::milliseconds(i * 100),
                     [i]
                     {
                        int p = ::getpid(); 
                        log_info(p, " ", i);
                     });
    }
    }
    /* auto now = std::chrono::steady_clock::now(); */
    /* base.runAt(now + std::chrono::seconds(10), */
    /*             [&now] */
    /*             { */
    /*                 log_info("once timer run..."); */
    /*                 log_info << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - now).count(); */ 
    /*             } */
    /*         ); */
    /* base.runEvery(now + std::chrono::seconds(5), */
    /*                 std::chrono::seconds(5), */
    /*                 [&now] */
    /*                 { */
    /*                     log_info("periodic timer run..."); */
    /*                     log_info <<  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - now).count(); */
    /*                 } */
    /*             ); */
    /* base.runAfter(std::chrono::seconds(7), */
    /*                 [&now] */
    /*                 { */
    /*                     log_info("after timer run..."); */
    /*                     log_info <<  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - now).count(); */
    /*                 } */
    /*             ); */
    
    base.loop();
    return 0;
}

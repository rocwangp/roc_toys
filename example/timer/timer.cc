#include "../../rtoys.h"

int main()
{
    rtoys::net::EventLoop base;
    auto now = std::chrono::steady_clock::now();
    base.runAt(now + std::chrono::seconds(10),
                [&now]
                {
                    log_info("once timer run...");
                    log_info << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - now).count(); 
                }
            );
    base.runEvery(std::chrono::seconds(5),
                    [&now]
                    {
                        log_info("periodic timer run...");
                        log_info <<  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - now).count();
                    }
                );
    base.runAfter(std::chrono::seconds(7),
                    [&now]
                    {
                        log_info("after timer run...");
                        log_info <<  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - now).count();
                    }
                );
    base.loop();
    return 0;
}

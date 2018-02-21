# rtoys

## 一个C++11实现的Linux平台下简单网络库，采用Reactor模式

### 基本组件设计原理

* 并发请求由epoll处理
* one loop per thread，每个线程一个epoll
* 线程采用std::thread



### 使用示例

#### 回显服务器

```c
#include "../../rtoys.h"

int main()
{
    rtoys::net::EventLoop base;
    rtoys::net::TcpServer server(&base, "localhost", 9999);
    server.onConnRead(
                    [](const auto& conn)
                    {
                        conn->send(conn->readAll());
                    }
                );
    base.loop();
    return 0;
}
```

#### 定时任务

>定时任务只能运行在一个特定的EventLoop中,如果配合服务器使用就只能运行在监听线程中(以前实现则是由服务器随机分配线程执行定时任务,导致定时任务不能分离出来必须依赖服务器使用)

```c
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
```

#### 连接器，通常用于客户端

>重连机制表示connect函数如果失败,则尝试在一定时间后重新链接.本质是设置定时任务,默认不开启

```c
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
```

------

## 不足及改进:

* 只是对网络请求的简单封装,主要是用于平时自己使用
* 在以后使用的过程中会根据新需求不断进行扩充

### 扩充计划有

* 支持UDP
* 支持断线重连(心跳机制)
* 支持protobuf
* 进行并发测试

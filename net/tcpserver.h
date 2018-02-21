#pragma once

#include "../util/noncopyable.h"
#include "../util/timer.h"
#include "connection.h"
#include "eventloop.h"

#include <memory>
#include <string>
#include <future>
#include <thread>
#include <unordered_map>
#include <vector>
#include <chrono>

namespace rtoys
{
    namespace net
    {
        class Acceptor;
        class EventLoop;


        class TcpServer : private util::noncopyable
        {
            public:
                typedef std::function<void(const std::shared_ptr<Connection>&)> conn_cb_type;

            public:
                TcpServer(EventLoop* loop, const std::string& ip, unsigned short port, int threadNums = 16);
                ~TcpServer(); 
                
                void onConnBuild(conn_cb_type cb) { connBuildCallBack_ = cb; }
                void onConnRead(conn_cb_type cb) { connReadCallBack_ = cb; }
                void onConnWrite(conn_cb_type cb) { connWriteCallBack_ = cb; }
                void onConnClose(conn_cb_type cb) { connCloseCallBack_ = cb; }

            private:
                void handleAccept(int fd);

            private:
                EventLoop* loop_;
                std::unique_ptr<Acceptor> acceptor_;
                conn_cb_type connBuildCallBack_;
                conn_cb_type connReadCallBack_;
                conn_cb_type connWriteCallBack_;
                conn_cb_type connCloseCallBack_;
                int nextLoop_;
                std::vector<std::thread> threads_;
                std::vector<EventLoop*> loops_;
                std::unordered_map<std::string, std::shared_ptr<Connection>> connMap_;
        };


    }
}

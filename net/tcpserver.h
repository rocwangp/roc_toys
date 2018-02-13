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
                typedef std::function<void(const std::shared_ptr<Connection>&)> conn_build_type;
                typedef std::function<void(const std::shared_ptr<Connection>&)> conn_read_type;
                typedef std::function<void(const std::shared_ptr<Connection>&)> conn_write_type;
                typedef std::function<void(const std::shared_ptr<Connection>&)> conn_close_type;

            public:
                TcpServer(const std::string& ip, unsigned short port, int threadNums = 16);
                TcpServer(EventLoop* loop, const std::string& ip, unsigned short port, int threadNums = 16);
                ~TcpServer(); 
                /* template <class F, class... Args> */
                /* auto onConnRead(F&& f, Args... args) */
                /*     -> std::future<typename std::result_of<F(Args...)>::type>; */
                
                void onConnBuild(conn_build_type cb) { connBuildCallBack_ = cb; }
                void onConnRead(conn_read_type cb) { connReadCallBack_ = cb; }
                void onConnWrite(conn_write_type cb) { connWriteCallBack_ = cb; }
                void onConnClose(conn_close_type cb) { connCloseCallBack_ = cb; }

                void start(int threadNums = 16);

                /* util::Timer runAt(const std::chrono::steady_clock::time_point& t, std::function<void()> cb); */

                /* util::Timer runEvery(const std::chrono::steady_clock::time_point& t, */ 
                /*               const std::chrono::milliseconds& interval, */ 
                /*               std::function<void()> cb); */

                /* util::Timer runAfter(const std::chrono::milliseconds& interval, std::function<void()> cb); */

                /* void cancel(const util::Timer& timer); */
            /* private: */
                /* util::Timer setTimer(const std::chrono::steady_clock::time_point& t, */
                /*                const std::chrono::milliseconds& interlva, */
                /*                std::function<void()> cb); */
            private:
                std::shared_ptr<EventLoop> loop_;
                std::unique_ptr<Acceptor> acceptor_;
                conn_build_type connBuildCallBack_;
                conn_read_type connReadCallBack_;
                conn_write_type connWriteCallBack_;
                conn_close_type connCloseCallBack_;
                int nextLoop_;
                std::vector<std::thread> threads_;
                std::vector<EventLoop*> loops_;
                std::unordered_map<std::string, std::shared_ptr<Connection>> connMap_;
        };


    }
}

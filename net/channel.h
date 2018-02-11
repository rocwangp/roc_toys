#pragma once

#include <memory>
#include <functional>

namespace rtoys
{
    namespace net
    {
        class EventLoop;

        class Channel
        {
            public:
                typedef std::function<void()> event_callback_type;
           
                enum State { INIT, ADDED, DELETED };
            public:
                explicit Channel(EventLoop* loop);
                Channel(EventLoop* loop, int fd);
                ~Channel();

            public:
                int fd();
                int events() { return events_; }
                
                void setActiveEvents(int activeEvents) { activeEvents_ = activeEvents; }
                bool isAdded() { return state_ == State::ADDED; }
                void setAdded() { state_ = State::ADDED; }
                void setDeleted() { state_ = State::DELETED; }

                void handleEvents();
            public:
                bool isReadEvent();
                bool isWriteEvent();
                bool isNoneEvent();

                bool readable();
                bool writeable();
                bool none();

                void enableRead(); 
                void enableWrite();
                void disableRead();
                void disableWrite();
                void disableAll();

                void update();

                void onRead(event_callback_type cb)   { readCallBack_ = cb; }
                void onWrite(event_callback_type cb)  { writeCallBack_ = cb; }
                void onClose(event_callback_type cb)  { closeCallBack_ = cb; }
            private:
                EventLoop *loop_;
                int fd_;
                int events_;
                int activeEvents_;
                State state_;
                
                event_callback_type readCallBack_;
                event_callback_type writeCallBack_;
                event_callback_type closeCallBack_;
                
        };
    }
}

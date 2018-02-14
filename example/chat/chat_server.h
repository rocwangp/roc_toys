#include "../../rtoys.h"

#include "chat_message.h"

#include <string>
#include <unordered_set>
#include <memory>
#include <deque>

using namespace rtoys::net;

class ChatRoom
{
    public:
        enum { MAX_RECORD_NUMS = 100 };
    public:
        ChatRoom() {}

        void join(const std::shared_ptr<Connection>& participant)
        {
            participants_.insert(participant);
        }

        void level(const std::shared_ptr<Connection>& participant)
        {
            participants_.erase(participant);
        }

        void deliver(const ChatMessage& message)
        {
            if(records_.size() >= MAX_RECORD_NUMS)
                records_.pop_front();
            records_.push_back(message);
            for(auto& participant : participants_)
                participant->send(message.data());
        }
    private:
        std::unordered_set<std::shared_ptr<Connection>> participants_; 
        std::deque<ChatMessage> records_;
};


class ChatServer
{
    public:
        ChatServer(unsigned short port = 9999)
            : server_(&base_, "localhost", port)
        {
            setCallBack(); 
        }

        void start()
        {
            base_.loop();
        }

        void setCallBack()
        {
            server_.onConnBuild(
                            [this](const auto& conn)
                            {
                               room_.join(conn); 
                            }
                        );
            server_.onConnRead(
                            [this](const auto& conn)
                            {
                                auto connMsg(conn->readAll());
                                ChatMessage chatMsg;
                                if(chatMsg.decodeMsg(connMsg))
                                {
                                    room_.deliver(chatMsg);
                                }
                            }
                        );
        }

    private:
        EventLoop base_;
        TcpServer server_;        
        ChatRoom room_;
};





#pragma once

#include <vector>
#include <sstream>
#include <string>


class ChatMessage
{
    public:
        enum { HEADER_SIZE = 4, MAX_BODY_SIZE = 1024 };
    public:
        ChatMessage()
            : bodyLen_(0)
        {  }

        char* data() { return &msg_[0]; }
        const char* data() const { return &msg_[0]; }

        char* header() { return &msg_[0]; }
        const char* header() const { return &msg_[0]; }

        char* body() { return &msg_[HEADER_SIZE]; }
        const char* body() const { return &msg_[HEADER_SIZE]; }
        
        std::size_t bodyLength() const { return bodyLen_; }
        void setBodyLength(std::size_t len) { bodyLen_ = len; }

        bool decodeMsg(const std::string& msg)
        {
            /* if(msg.size() < HEADER_SIZE) */
            /*     return false; */
            /* std::stringstream oss(msg.substr(0, HEADER_SIZE)); */
            /* oss >> bodyLen_; */
            /* if(bodyLen_ <= 0 || bodyLen_ > MAX_BODY_SIZE) */
            /*   return false; */
            msg_.assign(msg.begin(), msg.end());
            msg_.emplace_back('\0');
            return true;
        }
    private:
        std::vector<char> msg_;
        std::size_t bodyLen_;
};

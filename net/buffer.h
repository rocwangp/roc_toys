#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <sstream>

namespace rtoys
{
    namespace net
    {
        template <std::size_t BufferSize = 4096>
        class BaseBuffer
        {
            public:
                BaseBuffer()
                    : buffer_(BufferSize),
                      readIdx_(0),
                      writeIdx_(0)
                {  }

                char* data() { return &buffer_[readIdx_]; }
                const char* data() const { return &buffer_[readIdx_]; }

                int size() const noexcept { return writeIdx_ - readIdx_; }
                bool empty() const noexcept { return readIdx_ == writeIdx_; }

                int readableBytes() const noexcept { return writeIdx_ - readIdx_; }
                int writeableBytes() const noexcept { return buffer_.size() - writeIdx_; }
                int recyclableBytes() const noexcept { return readIdx_; }

                char* begin() noexcept { return &buffer_[readIdx_]; }
                char* end() noexcept { return &buffer_[writeIdx_]; }


                void append(const std::string& msg)
                {
                    enableSpace(msg.size()); 
                    std::copy(msg.begin(), msg.end(), end());
                    appendBytes(msg.size());
                }
                void append(const char* msg)
                {
                    int n = ::strlen(msg);
                    enableSpace(n);
                    std::copy(msg, msg + n, end());
                    appendBytes(n);
                }
                void append(int n)
                {
                    append(static_cast<long long int>(n));
                }
                void append(long long int n)
                {
                    std::stringstream oss;
                    oss << n;
                    append(oss.str());
                }
                void appendBytes(int bytes)
                {
                    writeIdx_ += bytes;
                }
                void enableSpace(int bytes)
                {
                    if(writeableBytes() > bytes)
                    {
                        return;
                    }
                    else if(recyclableBytes() + writeableBytes() > bytes)
                    {
                        std::move_backward(begin(), end(), buffer_.begin());
                        writeIdx_ = size();
                        readIdx_ = 0;
                    }
                    else
                    {
                        buffer_.resize(buffer_.size() + bytes);
                    }
                }

                void reset()
                {
                    readIdx_ = writeIdx_ = 0;
                }
            public:
                std::string retrieveAll()
                {
                    std::string msg(begin(), end());
                    reset();
                    return msg;
                }
            private:
                std::vector<char> buffer_;
                int readIdx_;
                int writeIdx_;

                static_assert(BufferSize > 0, "buffer can't be empty!");
        };

        typedef BaseBuffer<4096> Buffer;
    }
}

#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace rtoys
{
    namespace util
    {
        class Slice
        {
            public:
                Slice(const std::string& msg, char sign)
                {
                    std::stringstream oss(msg);
                    std::string line;
                    while(std::getline(oss, line, sign))
                    {
                        slices_.emplace_back(std::move(line));    
                    }
                }

            public:
                std::size_t size() const 
                {
                    return slices_.size();
                }

                std::string operator[](int pos)
                {
                    return slices_[pos];
                }

            private:
                std::vector<std::string> slices_;

        };

        template <class... Args>
        std::string format(const std::string& src, Args&&... args)
        {
            char buffer[1024] = "\0";
            std::sprintf(buffer, src.data(), std::forward<Args>(args)...);
            return std::string(buffer);
        }
    }
}

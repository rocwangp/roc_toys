#pragma once

#include <unistd.h>
#include <fcntl.h>
namespace rtoys
{
    namespace util
    {
        class io
        {
            public:
                static int open(const char* str)
                {
                    return ::open(str, O_RDONLY | O_CLOEXEC);    
                }
                static void close(int fd)
                {
                    ::close(fd);
                }
        };
    }
}

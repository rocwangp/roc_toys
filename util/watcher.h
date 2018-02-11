#pragma once

#include <unistd.h>

namespace rtoys
{
    namespace util
    {
        class PipeWatcher 
        {
            public:
                PipeWatcher()
                {
                    ::pipe(fd_);
                }
                ~PipeWatcher()
                {
                    ::close(fd_[0]);
                    ::close(fd_[1]);
                }

                void notify()
                {
                    ::write(fd_[1], "0", 1);
                }
                void clear()
                {
                    int value = 0;
                    ::read(fd_[0], &value, sizeof(value));
                }

                int readFd() 
                {
                    return fd_[0];
                }
            private:
                int fd_[2];

        };
    }
}

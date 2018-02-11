#pragma once


namespace rtoys
{
    namespace util
    {
        class noncopyable
        {
            public:
                noncopyable() {}
                noncopyable(const noncopyable&) = delete;
                noncopyable& operator=(const noncopyable&) = delete;
        };
    }
}

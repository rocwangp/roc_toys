#pragma once

#include <string>
#include <functional>

#include "../net/buffer.h"

namespace rtoys
{
    namespace util
    {

        class LogStream
        {
            public:
                typedef LogStream Self;

            public:
                Self& operator<<(const std::string& msg);
                Self& operator<<(const char* msg);
                Self& operator<<(int n);
                Self& operator<<(long long int n);
                Self& operator<<(std::size_t n);

            public:
                std::string retrieveAll();
            private:
                net::BaseBuffer<1024> buffer_;
        };

        class Logging
        {
            public:
                enum class LogLevel 
                {
                    TRACE,
                    DEBUG,
                    ERROR,
                    FATAL
                };

                typedef std::function<void(const std::string& msg)> output_func_type;
                typedef std::function<void()> flush_func_type;
            public:
                Logging(LogLevel level,
                        const std::string& filename,
                        const std::string& funcname,
                        int line);

                ~Logging();


                LogStream& stream() { return stream_; }
                
                
                static void setOutputFunc(output_func_type func);
                static void setFlushFunc(flush_func_type func);
                    
            private:
                void formatLevel();
                void formatThread();
                void formatTime();
            private:
                LogLevel level_;
                std::string filename_;
                std::string funcname_;
                int line_;
                LogStream stream_;

                static output_func_type kOutputFunc;
                static flush_func_type kFlushFunc;
        };
    }

#define log_trace   rtoys::util::Logging(rtoys::util::Logging::LogLevel::TRACE, __FILE__, __func__, __LINE__).stream()
#define log_debug   rtoys::util::Logging(rtoys::util::Logging::LogLevel::DEBUG, __FILE__, __func__, __LINE__).stream()
#define log_error   rtoys::util::Logging(rtoys::util::Logging::LogLevel::ERROR, __FILE__, __func__, __LINE__).stream()
#define log_fatal   rtoys::util::Logging(rtoys::util::Logging::LogLevel::FATAL, __FILE__, __func__, __LINE__).stream()
}

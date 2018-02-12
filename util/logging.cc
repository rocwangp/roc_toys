#include "logging.h"
#include <chrono>
#include <thread>

#include <unistd.h>

namespace rtoys
{
    namespace util
    {



        Logging::output_func_type Logging::kOutputFunc = 
                                [](const std::string& msg)
                                {
                                    ::fwrite(msg.c_str(), 1, msg.size(), stdout);
                                };

        Logging::flush_func_type Logging::kFlushFunc = 
                                []()
                                {
                                    ::fflush(stdout);
                                };

        void Logging::setOutputFunc(output_func_type func)
        {
            kOutputFunc = func;
        }

        void Logging::setFlushFunc(flush_func_type func)
        {
            kFlushFunc = func;
        }

        Logging::Logging(LogLevel level, 
                         const std::string& filename,
                         const std::string& funcname,
                         int line)
            : level_(level),
              filename_(filename),
              funcname_(funcname),
              line_(line)
        {
            formatTime();
            formatLevel();
            formatThread();
            formatPosition();
            if(stream_.size() < HEADER_SIZE)
                stream_ << std::string(HEADER_SIZE - stream_.size(), '-');
        }

        Logging::~Logging()
        {
            stream_ << "\n";
            kOutputFunc(stream_.retrieveAll()); 
            kFlushFunc();
            if(level_ == LogLevel::FATAL)
                ::abort();
        }

        void Logging::formatLevel()
        {
            switch(level_)
            {
                case LogLevel::TRACE:
                    stream_ << "Trace--";
                    break;
                case LogLevel::INFO:
                    stream_ << "Info---";
                    break;
                case LogLevel::DEBUG:
                    stream_ << "Debug--";
                    break;
                case LogLevel::ERROR:
                    stream_ << "Error--";
                    break;
                case LogLevel::FATAL:
                    stream_ << "Fatal--";
                    break;
            }
        }
        
        void Logging::formatThread()
        {
            std::stringstream oss;
            oss << std::this_thread::get_id();
            long long int tid;
            oss >> tid;
            stream_ << tid << "--";
        }

        void Logging::formatTime()
        {
            auto now = std::chrono::system_clock::now();
            std::time_t tt = std::chrono::system_clock::to_time_t(now);
            char *buf = ::ctime(&tt);
            buf[::strlen(buf) - 1] = '\0';
            stream_ << buf << "--";
        }

        void Logging::formatPosition()
        {
            stream_ << filename_ << " " << funcname_ << ":" << line_; 
        }




        std::size_t LogStream::size() const
        {
            return buffer_.size();
        }
        std::string LogStream::retrieveAll()
        {
            return buffer_.retrieveAll();
        }

        LogStream& LogStream::operator<<(const std::string& msg)
        {
            buffer_.append(msg);
            return *this;
        }

        LogStream& LogStream::operator<<(const char* msg)
        {
            buffer_.append(msg);
            return *this;
        }

        LogStream& LogStream::operator<<(int n)
        {
            buffer_.append(n);
            return *this;
        }
        LogStream& LogStream::operator<<(long n)
        {
            buffer_.append(static_cast<int>(n));
            return *this;
        }

        LogStream& LogStream::operator<<(long long int n)
        {
            buffer_.append(n);
            return *this;
        }

        LogStream& LogStream::operator()(const std::string& msg)
        {
            buffer_.append(msg);
            return *this;
        }

        LogStream& LogStream::operator()(const char* msg)
        {
            buffer_.append(msg);
            return *this;
        }

        LogStream& LogStream::operator()(int n)
        {
            buffer_.append(n);
            return *this;
        }
        LogStream& LogStream::operator()(long n)
        {
            buffer_.append(static_cast<int>(n));
            return *this;
        }

        LogStream& LogStream::operator()(long long int n)
        {
            buffer_.append(n);
            return *this;
        }

    }
}

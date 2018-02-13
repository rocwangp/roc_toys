#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <cstdint>
#include <string>
#include <cstring>
#include <memory>
#include <sstream>

#include "../net/buffer.h"
#include "../util/logging.h"

namespace rtoys
{
    namespace ip
    {
        namespace tcp
        {
            namespace address
            {
                
                class AddressBase
                {
                    public:
                        AddressBase() {}

                        virtual ~AddressBase() {}
                        virtual struct sockaddr* data() = 0;
                        virtual std::size_t size() = 0;
                        virtual void toPeerAddress(int fd) = 0;
                        virtual void toLocalAddress(int fd) = 0;
                        virtual std::string toString() = 0;
                    protected:
                        union sockaddress
                        {
                            struct sockaddr_in sockaddr_v4;
                            struct sockaddr_in6 sockaddr_v6;
                        };
                        sockaddress sockaddr_;

                };
                class v4 : public AddressBase
                {
                    public:
                        v4()
                        {

                        }
                        v4(const std::string& ip, unsigned short port)
                        {
                            sockaddr_.sockaddr_v4.sin_family = AF_INET;
                            sockaddr_.sockaddr_v4.sin_port = ::htons(port);
                            ::inet_pton(AF_INET, ip.c_str(), &sockaddr_.sockaddr_v4.sin_addr);
                        }

                        virtual ~v4() {}
                        virtual struct sockaddr* data() override
                        {
                            return (struct sockaddr*)(&sockaddr_.sockaddr_v4);
                        }
                        virtual std::size_t size() override
                        {
                            return sizeof(struct sockaddr_in);
                        }

                    public:
                        virtual void toPeerAddress(int fd) override
                        {
                            std::memset(&sockaddr_.sockaddr_v4, 0, sizeof(sockaddr_.sockaddr_v4));
                            socklen_t len = sizeof(sockaddr_.sockaddr_v4);
                            ::getpeername(fd, (struct sockaddr*)(&sockaddr_.sockaddr_v4), &len);
                        }

                        virtual void toLocalAddress(int fd) override
                        {
                            std::memset(&sockaddr_.sockaddr_v4, 0, sizeof(sockaddr_.sockaddr_v4));
                            socklen_t len = sizeof(sockaddr_.sockaddr_v4);
                            ::getsockname(fd, (struct sockaddr*)(&sockaddr_.sockaddr_v4), &len);
                        }
                        
                        virtual std::string toString() override
                        {
                            char ip[1024] = "\0";
                            char port[1024] = "\0";
                            std::sprintf(port, "%d", ::ntohs(sockaddr_.sockaddr_v4.sin_port));
                            ::inet_ntop(AF_INET, &sockaddr_.sockaddr_v4.sin_addr, ip, sizeof(ip));
                            std::stringstream oss;
                            oss << "<" << ip << ":" << port << ">";
                            return oss.str();
                        }
                        
                };
            }
            class socket
            {
                public:
                    enum SOCKET_STATE { BLOCK, NON_BLOCK };
                    enum {MAX_LISTEN_NUMS = 1000};
                    static int create_socket(SOCKET_STATE state = NON_BLOCK)
                    {
                        if(state == SOCKET_STATE::NON_BLOCK)
                            return nonblock_socket();
                        else
                            return ::socket(AF_INET, SOCK_STREAM, 0);
                    }
                    static int nonblock_socket()
                    {
                        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
                        int opt = 1;
                        ::setsockopt(fd, SOL_SOCKET, SOCK_NONBLOCK, &opt, sizeof(opt));
                        return fd;
                    }
                    static void close(int fd)
                    {
                        ::close(fd);
                    }

                    static int accept(int listenFd)
                    {
                        address::v4 v4Addr;
                        socklen_t len = v4Addr.size();
                        return ::accept(listenFd, v4Addr.data(), &len);
                    }

                    static bool bind(int fd, std::shared_ptr<address::AddressBase> addressPtr)
                    {
                        if(::bind(fd, addressPtr->data(), addressPtr->size()) == -1)
                            return false;
                        return true;
                    }
                    static bool listen(int fd, std::size_t nums = MAX_LISTEN_NUMS)
                    {
                        if(::listen(fd, nums) == -1)
                            return false;
                        return true;
                    }

                    static bool connect(int fd, const std::string& ip, unsigned short port)
                    {
                        address::v4 v4Addr(ip, port);
                        if(::connect(fd, v4Addr.data(), v4Addr.size()) == -1)
                            return false;
                        return true;
                    }

                    static int read(int fd, std::shared_ptr<net::Buffer> readBuffer)
                    {
                        unsigned int bytes = 0;
                        ::ioctl(fd, FIONREAD, &bytes); 
                        readBuffer->enableSpace(bytes);
                        int n = ::read(fd, readBuffer->end(), bytes);
                        return n;
                    }

                    static int write(int fd, std::shared_ptr<net::Buffer> writeBuffer)
                    {
                        return ::write(fd, writeBuffer->begin(), writeBuffer->readableBytes()); 
                    }

                    static int write(int fd, const std::string& msg)
                    {
                        return ::write(fd, msg.c_str(), msg.size());
                    }

                    static void reuse_port(int fd)
                    {
                        int opt = 1;
                        ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
                    }
                    static void reuse_address(int fd)
                    {
                        int opt = 1;
                        ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
                    }
                    static void set_nonblock(int fd)
                    {
                        int opt = 1;
                        ::setsockopt(fd, SOL_SOCKET, SOCK_NONBLOCK, &opt, sizeof(opt));
                    }
            };

            
            

            class endpoint
            {
                public:
                    endpoint()
                        : fd_(-1)
                    {

                    }
                    endpoint(address::v4, int fd)
                        : fd_(fd),
                          peerAddr_(std::make_shared<address::v4>()),
                          localAddr_(std::make_shared<address::v4>())
                    {
                       peerAddr_->toPeerAddress(fd_); 
                       localAddr_->toLocalAddress(fd_);
                    }

                    void reset(address::v4 addr, int fd)
                    {
                        endpoint tmp(addr, fd);
                        std::swap(tmp, *this);
                    }
                    
                    std::string peerAddrToString()
                    {
                        return peerAddr_->toString(); 
                    }
                    std::string localAddrToString()
                    {
                        return localAddr_->toString();
                    }

                private:
                    int fd_;
                    std::shared_ptr<address::AddressBase> peerAddr_;
                    std::shared_ptr<address::AddressBase> localAddr_;
            };
        }
    }
}

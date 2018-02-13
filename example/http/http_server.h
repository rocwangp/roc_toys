#pragma once

#include "../../net/tcpserver.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <sstream>
#include <iostream>

struct Request
{
    std::string method, source, version;
    std::unordered_map<std::string, std::string> headers;
    std::shared_ptr<std::stringstream> content;
    std::smatch match;
};

class HttpServer
{
    public:
        typedef std::shared_ptr<rtoys::net::Connection> conn_pointer;
        typedef std::map<std::string,
                         std::unordered_map<std::string,
                                            std::function<void(std::stringstream&, const Request&)>>> resource_type;

    public:
        HttpServer(const std::string& ip, unsigned short port)
            : server_(ip, port)
        {

        }

        void start()
        {
            for(auto it = resource_.begin(); it != resource_.end(); ++it)
                resources_.emplace_back(it);
            for(auto it = defaultResource_.begin(); it != defaultResource_.end(); ++it)
                resources_.emplace_back(it);

            server_.onConnRead(
                        [this](const conn_pointer& conn)
                        {
                            log_trace;
                            std::stringstream stream(conn->readUtil("\r\n\r\n"));
                            Request request = parseRequest(stream);
                            /* request.context = std::make_shared<std::stringstream>(stream); */
                            request.content = std::make_shared<std::stringstream>(conn->readAll());
                            respond(conn, request);
                            /* if(!request.headers.count("Connection") || request.headers["Connection"] != "keep-alive") */
                            /* { */
                            /*     conn->close(); */
                            /* } */
                        }
                    );

            server_.start();
        }

        resource_type& resource() { return resource_; }
        resource_type& defaultResource() { return defaultResource_; }

    private:
        Request parseRequest(std::stringstream& stream)
        {
            std::regex e("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
            std::smatch match;

            Request request;
            std::string line;
            std::getline(stream, line);
            line.pop_back();
            if(std::regex_match(line, match, e))
            {
                request.method = match[1];
                request.source = match[2];
                request.version = match[3];
                
                log_debug << match[1] << " " << match[2] << " " << match[3];
                
                e = "^([^:]*): ?(.*)$";
                bool matched = false;
                do
                {
                    std::getline(stream, line);
                    line.pop_back();
                    matched = std::regex_match(line, match, e);
                    if(matched)
                        request.headers[match[1]] = match[2];
                }while(matched);
            }
            return request;
        }

        void respond(const conn_pointer& conn, Request& request)
        {
            for(auto it : resources_)
            {
                std::smatch match;
                std::regex e(it->first);
                if(std::regex_match(request.source, match, e) && it->second.count(request.method))
                {
                    request.match = std::move(match); 
                    std::stringstream response; 
                    it->second[request.method](response, request);
                    log_debug << response.str();
                    conn->send(response.str()); 
                    break;
                }
            }
        }

    private:
        resource_type resource_;
        resource_type defaultResource_;
    private:
        rtoys::net::TcpServer server_;
        std::vector<resource_type::iterator> resources_;
        
};

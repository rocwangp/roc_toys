#pragma once

#include "http_server.h"
#include <fstream>

void startServer(HttpServer& server)
{
    server.resource()["^/string/?$"]["POST"] = 
        [](std::stringstream& response, const Request& request)
        {
            log_trace;
            std::string content = request.content->str();
            response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.size()
                     << "\r\n\r\n" << content;
        };

    server.resource()["^/info/?$"]["GET"] = 
        [](std::stringstream& response, const Request& request)
        {
            log_trace;
            std::stringstream contentStream;
            contentStream << "<h1>Request:</h1>";
            contentStream << request.method << " " << request.source << " HTTP/" << request.version << "<br>";
            for(auto& header : request.headers)
                contentStream << header.first << ": " << header.second << "</br>";

            contentStream.seekp(0, std::ios::end);
            response << "HTTP/1.1 200 OK\r\nContentLength: " << contentStream.tellp()
                     << "\r\n\r\n" << contentStream.rdbuf();
        };

    server.resource()["^/match/([0-9a-zA-Z]+)/?$"]["GET"] = 
        [](std::stringstream& response, const Request& request)
        {
            log_trace;
            std::string content = request.match[1];
            response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.size() 
                     << "\r\n\r\n" << content;
        };

    server.defaultResource()["^/?(.*)$"]["GET"] =
        [](std::stringstream& response, const Request& request)
        {
            log_trace;
            std::string filename = "web/";
            std::string path = request.match[1];

            std::size_t lastPos = path.rfind('.');
            std::size_t currentPos = 0;
            std::size_t pos = 0;
            while((pos = path.find_first_of('.', currentPos)) != std::string::npos && pos != lastPos)
            {
                currentPos = pos;
                path.erase(pos, 1);
                --lastPos;
            }

            filename.append(path);
            if(filename.find('.') == std::string::npos)
            {
                if(filename.back() != '/')
                    filename.append(1, '/');
                filename.append("/index.html");
            }

            std::ifstream ifs(filename, std::ios_base::in);
            if(ifs.is_open())
            {
                ifs.seekg(0, std::ios::end);
                std::size_t len = ifs.tellg();
                ifs.seekg(0, std::ios::beg);
                response << "HTTP/1.1 200 OK\r\nContent-Length: " << len 
                         << "\r\n\r\n" << ifs.rdbuf();
                ifs.close();
            }
            else
            {
                std::string content = "could not open file " + filename;
                response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.size()
                         << "\r\n\r\n" << content;
            }
        };

    server.start();
}

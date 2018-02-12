#pragma once

#include "http_server.h"

void startServer(HttpServer& server)
{
    server.resource()["^/string/?$"]["POST"] = 
        [](std::stringstream& response, const Request& request)
        {
            std::string content = request.content.str();
            response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.size()
                     << "\r\n\r\n" << content;
        };

    server.resource()["^/info/?$"]["GET"] = 
        [](std::stringstream& response, const Request& request)
        {
            std::stringstream contentStream;
            contentStream << "<h1>Request:</h1>";
            contentStream << request.method << " " << request.source << " HTTP/" << request.version << "<br>";
            for(auto& header : request.headers)
                contentStream << header.first << ": " << header.second << "</br>";

            contentStream.seekp(0, std::ios::end);
            response << "HTTP/1.1 200 OK\r\nContentLength: " << contentStream.tellp()
                     << "\r\n\r\n" << contentStream.rdbuf();
        };
}

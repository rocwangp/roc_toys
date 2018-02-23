#pragma once

#include "http_server.h"
#include <fstream>
#include <wait.h>

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
            contentStream << request.method << " " << request.uri << " HTTP/" << request.version << "<br>";
            for(auto& header : request.headers)
                contentStream << header.first << ": " << header.second << "</br>";

            contentStream.seekp(0, std::ios::end);
            response << "HTTP/1.1 200 OK\r\n"
                     << "Content-Length: " << contentStream.tellp() << "\r\n"
                     << "Content-Type: " << "text/plain"
                     << "\r\n\r\n" << contentStream.rdbuf();
        };

    server.resource()["^/match/([0-9a-zA-Z]+)/?$"]["GET"] = 
        [](std::stringstream& response, const Request& request)
        {
            log_trace;
            std::string content = request.match[1];
            response << "HTTP/1.1 200 OK\r\n"
                     << "Content-Length: " << content.size() << "\r\n"
                     << "Content-Type: " << "text/plain" 
                     << "\r\n\r\n" << content;
        };

    server.resource()["^/cgi-bin/([0-9a-zA-Z]+)\\?(.*)$"]["GET"] = 
        [](std::stringstream& response, const Request& request)
        {
            log_trace;
            int fds[2];
            ::pipe(fds);
            pid_t pid = 1;
            if((pid = ::fork()) == 0)
            {
                ::close(fds[0]);

                char path[1024] = "\0";
                ::getcwd(path, sizeof(path));
                ::strcat(path, "/cgi-bin/");
                std::string filename = path + request.match[1].str();
                char* emptyList[] = { nullptr };
                ::setenv("QUERY_STRING", request.match[2].str().c_str(), 1);
                ::dup2(fds[1], STDOUT_FILENO);
                ::execve(filename.c_str(), emptyList, ::environ);
            }
            else
            {
                ::close(fds[1]);
                char buffer[4096] = "\0";
                int n = 0;
                while((n = ::read(fds[0], buffer, sizeof(buffer))) > 0)
                {
                    buffer[n] = '\0';
                    response << buffer;
                }
                ::waitpid(pid, nullptr, 0);
                ::close(fds[0]);
            }
        };

    server.resource()["^/cgi-bin/([0-9a-zA-Z]+)$"]["POST"] = 
        [](std::stringstream& response, const Request& request)
        {
            log_trace;
            int responseFds[2];
            int contentFds[2];
            ::pipe(responseFds);
            ::pipe(contentFds);

            pid_t pid = 1;
            if((pid = ::fork()) == 0)
            {
                ::close(responseFds[0]);
                ::close(contentFds[1]);

                char path[1024] = "\0";
                ::getcwd(path, sizeof(path));
                ::strcat(path, "/cgi-bin/");
                std::string filename = path + request.match[1].str();
                char* emptyList[] = { nullptr };

                ::setenv("CONTENT-LENGTH", request.headers.at("Content-Length").c_str(), 1);
                ::dup2(contentFds[0], STDIN_FILENO);
                ::dup2(responseFds[1], STDOUT_FILENO);
                ::execve(filename.c_str(), emptyList, ::environ);
            }
            else
            {
                ::write(contentFds[1], request.content->str().c_str(), request.content->str().size());

                ::close(responseFds[1]);
                ::close(contentFds[0]);
                ::close(contentFds[1]);

                char buffer[4096] = "\0";
                int n = 0;
                while((n = ::read(responseFds[0], buffer, sizeof(buffer))) > 0)
                {
                    buffer[n] = '\0';
                    response << buffer;
                }
                ::waitpid(pid, nullptr, 0);
                ::close(responseFds[0]);
            }
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

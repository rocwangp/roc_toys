#include <iostream>
#include <sstream>
#include <cstdlib>

int main()
{
    char* buf = ::getenv("QUERY_STRING");
    int n1 = 0, n2 = 0;
    char flag;
    if(buf)
    {
        std::stringstream oss(buf);
        oss >> n1 >> flag >> n2;
    }

    std::stringstream response;
    std::stringstream content;
    content << "QUERY_STRING=" << buf;
    content << "Welcome to add.com: THE Internet addition portal.\r\n<p>";
    content << "The answer is: " << n1 << " + " << n2 << " = " << n1 + n2 << "\r\n<p>";
    content << "Thanks for visiting!\r\n";

    response << "HTTP/1.1 200 OK\r\n"
             << "Connection: close\r\n"
             << "Content-Length: " << content.str().size() << "\r\n"
             << "Content-Type: text/html\r\n\r\n"
             << content.rdbuf();
    std::cout << response.rdbuf();
    std::cout.flush();
    return 0;
}

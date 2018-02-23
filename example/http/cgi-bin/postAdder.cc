#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>
#include <cstdlib>


int main()
{
    char* buf = ::getenv("CONTENT-LENGTH");
    std::stringstream content;
    if(buf == nullptr)
    {
        std::cout << "HTTP/1.1 404 NOT FOUND\r\n";
        content << "Something is wrong\r\n";
    }
    else
    {
        std::cout << "HTTP/1.1 200 OK\r\n";
        int n1, n2;
        std::cin >> n1 >> n2;
        content << n1 << " + " << n2 << " = " << n1 + n2;
    }
    std::cout << "Content-Length: " << content.str().size() << "\r\n"
              << "Content-Type: " << "test/html\r\n\r\n"
              << content.rdbuf();
    std::cout.flush();
    return 0;
     
}

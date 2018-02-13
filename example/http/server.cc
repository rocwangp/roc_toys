#include "http_server.h"
#include "handler.h"

int main()
{
    HttpServer server("localhost", 9999); 
    startServer(server);
    return 0;
}

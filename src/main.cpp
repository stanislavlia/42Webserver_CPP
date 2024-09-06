#include "Server.hpp"
#include "Webserver.hpp"

int main()
{
    struct sockaddr_in address;

    Server  server(&address, PORT);

    server.setup_server();
    server.run();

};
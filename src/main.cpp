#include "Webserver.hpp"

#define PORT 8012
#define BUFF_SIZE 1024

int create_server_fd()
{
    int server_fd;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        std::cerr << "Socket creation failed\n";
        exit(EXIT_FAILURE); 
    }
    return server_fd;
};


void set_socket_options(int server_fd, int opt)
{
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0)
    {
        std::cerr << "setsockopt failed\n";
        close(server_fd);
        exit(EXIT_FAILURE);
    }
}


void setup_socketaddress(struct sockaddr_in *sock_address, int host, int port)
{
    sock_address->sin_family = AF_INET;
    sock_address->sin_addr.s_addr = host;
    sock_address->sin_port = htons(port);
};



int main()
{
    int server_fd = create_server_fd();
    int opt = 1;

    set_socket_options(server_fd, opt);

    struct sockaddr_in address;
    setup_socketaddress(&address, INADDR_ANY, PORT);

    std::cout << "Server socket created and configured successfully.\n";
    std::cout << "Server FD: " << server_fd << std::endl;

    // Bind, listen, and accept would follow here in a full implementation.

    return 0;
}

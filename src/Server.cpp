#include "Server.hpp"

Server::Server()
{

};

Server::Server(struct sockaddr_in  *sock_address, int port) : _sock_address(sock_address), _port(port)
{
    std::cout << "Server initialized\n";
};


Server::~Server()
{

};


//==================SERVER SETUP=====================
void    Server::_create_server_socket()
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (_server_fd == -1)
    {    //throw exception later
        std::cerr << "Socket creation failed\n";
        exit(EXIT_FAILURE); 
    };
    // Set the server socket to non-blocking mode
    fcntl(_server_fd, F_SETFL, O_NONBLOCK);
};


void Server::_set_socket_options(int opt)
{
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0)
    {
        std::cerr << "setsockopt failed\n"; //replace with exceptions later
        close(_server_fd);
        exit(EXIT_FAILURE);
    }
}

void Server::_setup_socketaddress(int host)
{
    _sock_address->sin_family = AF_INET;
    _sock_address->sin_addr.s_addr = host;
    _sock_address->sin_port = htons(_port);
};





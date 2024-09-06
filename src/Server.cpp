#include "Server.hpp"

Server::Server()
{

};

Server::Server(struct sockaddr_in  *sock_address, int port) 
: _port(port), _sock_address(sock_address) 
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


void    Server::_bind_socket()
{
    if (bind(_server_fd, (struct sockaddr *)_sock_address, sizeof(*_sock_address)) < 0)
    {
        std::cerr << "Failed to bind socket" << std::endl;
        close(_server_fd);
        exit(EXIT_FAILURE);
    }
};

void    Server::_listen_socket()
{
    if (listen(_server_fd, CONN_QUEUE) < 0)
    {
        std::cerr << "Failed to listen\n";
        close(_server_fd);
        exit(EXIT_FAILURE);
    }
};

int    Server::_accept_connection()
{
    int new_socket;
    socklen_t addrlen = sizeof(*_sock_address);

    new_socket = accept(_server_fd, (struct sockaddr*) _sock_address, &addrlen);
    //Add some error handling & exceptions
    return new_socket;
};



void    Server::setup_server()
{
    _create_server_socket();
    _set_socket_options(1);
    _setup_socketaddress(INADDR_ANY);
    
    FD_ZERO(&read_fds);
    FD_SET(_server_fd, &read_fds);

    std::cout << "Server socket created and configured successfully.\n";
    std::cout << "Server FD: " << _server_fd << std::endl;

    _bind_socket();
    _listen_socket();
    std::cout << "Listening on port: " << PORT << std::endl;

};
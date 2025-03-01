#include "Server.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"
#include "sstream"
#include <sys/stat.h>
#include <dirent.h>


// Server::Server(Monitor* mon, const std::vector<ServerParam>& server_param)
// : monitor(mon), configs(server_param)
// {
//     Logger::logMsg(DEBUG, "Server initialized");
//     _host_to_port = std::map<std::string, int>();
//     for (size_t i = 0; i < server_param.size(); i++)
//     {
//         _ports.push_back(server_param[i].getPort());
//         _host = server_param[0].getHost();
//         std::cout << "HOST: " << _host << std::endl;
//         std::cout << "PORT: " << _ports[i] << std::endl;
//     }
// }

Server::Server(Monitor* mon, const std::vector<ServerParam>& server_param)
: monitor(mon), configs(server_param)
{
    Logger::logMsg(DEBUG, "Server initialized");
    _host_to_port = std::map<std::string, int>();

    for (size_t i = 0; i < server_param.size(); i++)
    {
        std::string host = server_param[i].getHost();
        int port = server_param[i].getPort();

        if (_host_to_port.find(host) != _host_to_port.end() && _host_to_port[host] == port)
        {
            _host_to_port.erase(host);
        }
        else
        {
            _host_to_port[host] = port;
            _ports.push_back(port);
            _host = host;
        }
    }
}

Server::~Server()
{
    for (size_t i = 0; i < _server_fds.size(); i++) {
        close(_server_fds[i]);
    }
}

void Server::_listen_socket(int server_fd)
{
    if (listen(server_fd, CONN_QUEUE) < 0)
    {
        close(server_fd);
        throw SocketHandlingException("Failed to listen");
    }
}

void Server::_set_socket_options(int opt, int server_fd)
{
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        close(server_fd);
        throw SocketHandlingException("setsockopt() failed");
    }
}

void Server::_setup_socketaddress(int port, int server_fd)
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, _host.c_str(), &address.sin_addr);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(server_fd);
        throw SocketHandlingException("Bind failed");
    }

    _sock_address.push_back(address);
}

int Server::_create_server_socket()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        throw SocketHandlingException("Socket creation failed");
    }
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    return server_fd;
}

void Server::setup_server()
{
    std::cout << "Setting up server" << std::endl;
    if (_ports.size() <= 0)
    {
        throw SocketHandlingException("No ports to listen on");
    }

    for (size_t i = 0; i < _ports.size(); i++)
    {
        // create server socket
        int server_fd = _create_server_socket();
        if (server_fd == -1)
        {
            Logger::logMsg(ERROR, "Failed to create server socket");
            continue;
        }

        _server_fds.push_back(server_fd);

        _set_socket_options(1, _server_fds[i]);

        _setup_socketaddress(_ports[i], _server_fds[i]);

        _listen_socket(_server_fds[i]);

        server_fd_to_port[_server_fds[i]] = _ports[i];

        monitor->addReadFd(_server_fds[i]);
    }
    if (_server_fds.empty())
    {
        throw SocketHandlingException("No server sockets created");
    }
}
#include "Server.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"
#include "sstream"
#include <sys/stat.h>
#include <dirent.h>

Server::Server()
{
};

Server::Server(struct sockaddr_in  *sock_address, const std::vector<ServerParam>& server_param) 
: configs(server_param) , _sock_address(sock_address)
{
	Logger::logMsg(DEBUG, "Server initialized");
	_port = server_param[0].getPort();
	_host = server_param[0].getHost().c_str();
};


Server::~Server()
{

};


//==================SERVER SETUP=====================
void    Server::_create_server_socket()
{
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (_server_fd == -1)
		throw SocketHandlingException("Socket creation failed");
	fcntl(_server_fd, F_SETFL, O_NONBLOCK);
};


void Server::_set_socket_options(int opt)
{
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0)
	{ 
		close(_server_fd);
		throw SocketHandlingException("setsockopt() failed");
	}
}

void	Server::_setup_socketaddress()
{
	_sock_address->sin_family = AF_INET;
	//_sock_address->sin_addr.s_addr = host;
	_sock_address->sin_port = htons(_port);
    inet_pton(AF_INET, _host.c_str(), &_sock_address->sin_addr);
};

void    Server::_bind_socket()
{
	if (bind(_server_fd, (struct sockaddr *)_sock_address, sizeof(*_sock_address)) < 0)
	{
		close(_server_fd);
		throw SocketHandlingException("Failed to bind");	
	}
};

void    Server::_listen_socket()
{
	if (listen(_server_fd, CONN_QUEUE) < 0)
	{
		close(_server_fd);
		throw SocketHandlingException("Failed to listen");		
	}
};

int    Server::_accept_connection()
{
	int new_socket;
	socklen_t addrlen = sizeof(*_sock_address);

	new_socket = accept(_server_fd, (struct sockaddr*) _sock_address, &addrlen);
	// Add some error handling & exceptions
	return new_socket;
};

void    Server::setup_server()
{
	_create_server_socket();
	_set_socket_options(1);
	_setup_socketaddress(); // 0.0.0.0 - special address that listen on all available net interfaces

	FD_ZERO(&read_fds);
	FD_SET(_server_fd, &read_fds);

	Logger::logMsg(INFO, "Server socket created and configured successfully");
	Logger::logMsg(DEBUG, "Server FD: %d", _server_fd);

	_bind_socket();
	_listen_socket();
	Logger::logMsg(DEBUG, "Listening on http://%s:%d", _host.c_str(), _port);

};

void	Server::run()
{

	int activity;
	int max_fd = _server_fd;
	int new_socket;
	int valread;

	bool is_running = true;
	char buffer[BUFF_SIZE] = {0};

	while (is_running)
	{
		fd_set current_fds = read_fds;

		activity = select(max_fd + 1, &current_fds, NULL, NULL, NULL);
		if (activity < 0)
			continue;

		if (FD_ISSET(_server_fd, &current_fds))
		{
			new_socket = _accept_connection();
			if (new_socket >= 0)
			{   
				FD_SET(new_socket, &read_fds);
				if (new_socket > max_fd)
					max_fd = new_socket;
				
				Logger::logMsg(INFO, "New connection accepted. SOCKET FD: %d", new_socket);
			}
		}

		for (int i = 0; i <= max_fd; i++)
		{
			if (FD_ISSET(i, &current_fds) && i != _server_fd) 
			{
				memset(buffer, 0, sizeof(buffer));
				valread = read(i, buffer, sizeof(buffer));
				if (valread == 0)
				{
					Logger::logMsg(INFO, "Client disconnected; SOCKET FD: %d", i);
					close(i);
					FD_CLR(i, &read_fds);
				}
				else if (valread > 0)
				{
					std::string buffer_str(buffer);
					Request	request(configs[0]);
					request.parseRequest(buffer);

					RequestHandler handler(i, request, configs[0]);
					handler.handleRequest();
				}
			}   
		}
	}
};

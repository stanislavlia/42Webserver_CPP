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

void	Server::_handle_new_connections(int& max_fd, fd_set& read_fds, int _server_fd)
{
	fd_set current_fds = read_fds;

	int activity = select(max_fd + 1, &current_fds, NULL, NULL, NULL);
	if (activity < 0)
		return;

	if (FD_ISSET(_server_fd, &current_fds))
	{
		int new_socket = _accept_connection();
		if (new_socket >= 0)
		{
			FD_SET(new_socket, &read_fds);
			if (new_socket > max_fd)
				max_fd = new_socket;

			Logger::logMsg(INFO, "New connection accepted. SOCKET FD: %d", new_socket);
		}
	}
}

bool	isRequestComplete(const std::string& request)
{
	// Check for the end of headers marker
	size_t headers_end = request.find("\r\n\r\n");
	if (headers_end == std::string::npos)
	{
		std::cout << "==========False 1============" << std::endl;
		return false; // Headers are not complete
	}

	// Extract headers
	std::string headers = request.substr(0, headers_end);

	// Check for Transfer-Encoding: chunked header
	size_t transfer_encoding_pos = headers.find("Transfer-Encoding: chunked");
	if (transfer_encoding_pos != std::string::npos)
	{
		// Check if the entire chunked body has been received
		size_t chunked_end = request.find("0\r\n\r\n", headers_end + 4);
		if (chunked_end == std::string::npos)
		{
			std::cout << "==========False 3============" << std::endl;
			return false; // Chunked body is not complete
		}
	}


	// Check for Content-Length header
	size_t content_length_pos = headers.find("Content-Length: ");
	if (content_length_pos != std::string::npos)
	{
		size_t content_length_end = headers.find("\r\n", content_length_pos);
		std::string content_length_str = headers.substr(content_length_pos + 16, content_length_end - content_length_pos - 16);
		
		// Convert content_length_str to size_t using std::istringstream
		std::istringstream iss(content_length_str);
		size_t content_length;
		iss >> content_length;

		// Check if the entire body has been received
		size_t body_start = headers_end + 4;
		size_t body_length = request.size() - body_start;
		std::cout << "Body length: " << body_length << std::endl;
		std::cout << "Content length: " << content_length << std::endl;
		if (body_length < content_length)
		{
			std::cout << "==========False 2============" << std::endl;
			return false; // Body is not complete
		}
	}

	std::cout << "==========True============" << std::endl;
	return true; // Request is complete
}

// Add this to maintain a buffer for each client
std::map<int, std::string> client_buffers;

void Server::run()
{
	int max_fd = _server_fd;
	char buffer[BUFF_SIZE] = {0};

	while (true)
	{
		_handle_new_connections(max_fd, read_fds, _server_fd);

		// Loop through all clients to handle their requests
		for (int i = 0; i <= max_fd; i++)
		{
			if (FD_ISSET(i, &read_fds) && i != _server_fd)
			{
				memset(buffer, 0, sizeof(buffer));
				int valread = read(i, buffer, sizeof(buffer));

				if (valread <= 0)
				{
					if (valread == 0)
					{
						Logger::logMsg(INFO, "Client disconnected; SOCKET FD: %d", i);
					}
					else
					{
						Logger::logMsg(ERROR, "Read error; SOCKET FD: %d", i);
					}
					close(i);
					FD_CLR(i, &read_fds);
					client_buffers.erase(i); // Remove the buffer for this client
					break;
				}

				// Append data to the client's buffer
				client_buffers[i].append(buffer, valread);
				std::cout << "Request in server core: " << client_buffers[i] << std::endl;
				// std::cout << "Request in server core: " << client_buffers[i] << std::endl;

				// Check if the request is complete
				if (isRequestComplete(client_buffers[i]))
				{
					// Process the complete request
					Request request(configs[0]);
					request.parseRequest(client_buffers[i].c_str());

					RequestHandler handler(i, request, configs[0]);
					handler.handleRequest();
					std::cout << "Request complete" << std::endl;

					// Clear the buffer for this client after processing
					client_buffers[i].clear();
					break;
				}
				else
				{
					std::cout << "Request not complete" << std::endl;
				}
			}
		}
	}
}

// void	Server::run()
// {
// 	int activity;
// 	int max_fd = _server_fd;
// 	int new_socket;
// 	int valread;

// 	bool is_running = true;
// 	// char buffer[BUFF_SIZE] = {0};

// 	while (is_running)
// 	{
// 		fd_set current_fds = read_fds;

// 		activity = select(max_fd + 1, &current_fds, NULL, NULL, NULL);
// 		if (activity < 0)
// 			continue;

// 		if (FD_ISSET(_server_fd, &current_fds))
// 		{
// 			new_socket = _accept_connection();
// 			if (new_socket >= 0)
// 			{
// 				FD_SET(new_socket, &read_fds);
// 				if (new_socket > max_fd)
// 					max_fd = new_socket;

// 				Logger::logMsg(INFO, "New connection accepted. SOCKET FD: %d", new_socket);
// 			}
// 		}
// 	}
// }
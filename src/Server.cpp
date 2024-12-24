#include "Server.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"
#include "sstream"
#include <sys/stat.h>
#include <dirent.h>

// Function to check if the request is complete
bool	isRequestComplete(const std::string& request) {
	// Check for the end of headers marker
	size_t headers_end = request.find("\r\n\r\n");
	if (headers_end == std::string::npos) {
		std::cout << "Headers are not complete" << std::endl;
		return false; // Headers are not complete
	}

	// Extract headers
	std::string headers = request.substr(0, headers_end);

	// Check for Transfer-Encoding: chunked header
	size_t transfer_encoding_pos = headers.find("Transfer-Encoding: chunked");
	if (transfer_encoding_pos != std::string::npos) {
		// Check if the entire chunked body has been received
		size_t chunked_end = request.find("0\r\n\r\n", headers_end + 4);
		if (chunked_end == std::string::npos) {
			std::cout << "Chunked body is not complete" << std::endl;
			return false; // Chunked body is not complete
		}
	}

	// Check for Content-Length header
	size_t content_length_pos = headers.find("Content-Length: ");
	if (content_length_pos != std::string::npos) {
		size_t content_length_end = headers.find("\r\n", content_length_pos);
		std::string content_length_str = headers.substr(content_length_pos + 16, content_length_end - content_length_pos - 16);

		// Convert content_length_str to size_t using std::istringstream
		std::istringstream iss(content_length_str);
		size_t content_length;
		iss >> content_length;

		// Check if the entire body has been received
		std::cout << "Request size: " << request.size() << std::endl;
		size_t body_start = headers_end + 4;
		std::cout << "Body start: " << body_start << std::endl;
		size_t body_length = request.size() - body_start;
		std::cout << "Body length: " << body_length << std::endl;
		std::cout << "Content length: " << content_length << std::endl;

		if (body_length < content_length) {
			std::cout << "Body is not complete" << std::endl;
			return false; // Body is not complete
		}
	}
	return true; // Request is complete
}

void	Server::addServerSocketsToReadFds(int& max_fd)
{
	for (size_t i = 0; i < _server_fds.size(); i++)
	{
		std::cout << "Server fd add to read fds : " << _server_fds[i] << std::endl;
		FD_SET(_server_fds[i], &read_fds);
		if (_server_fds[i] > max_fd)
		{
			max_fd = _server_fds[i];
		}
	}
}

void	Server::addClientSocketsToReadFds(const std::vector<int>& client_fds, int& max_fd)
{
	for (size_t i = 0; i < client_fds.size(); i++)
	{
		if (client_fds[i] > 0)
		{
			std::cout << "Client fd add to read fds : " << client_fds[i] << std::endl;
			FD_SET(client_fds[i], &read_fds);
		}
		if (client_fds[i] > max_fd)
		{
			max_fd = client_fds[i];
		}
	}
}

void	Server::addClientSocketsToWriteFds(const std::vector<int>& client_fds, int& max_fd)
{
	for (size_t i = 0; i < client_fds.size(); i++)
	{
		if (client_fds[i] > 0)
		{
			std::cout << "Client fd add to write fds : " << client_fds[i] << std::endl;
			FD_SET(client_fds[i], &write_fds);
		}
		if (client_fds[i] > max_fd)
		{
			max_fd = client_fds[i];
		}
	}
}

void	Server::handleNewConnections(std::vector<int>& client_fds, std::map<int, int>& client_fd_to_port, int& max_fd)
{
	for (size_t i = 0; i < _server_fds.size(); i++)
	{
		int server_fd = _server_fds[i];
		if (FD_ISSET(server_fd, &copy_read_fds))
		{

			struct sockaddr_in client_address;
			int addrlen = sizeof(client_address);
			int new_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*)&addrlen);
			if (new_socket < 0)
			{
				switch (errno)
				{
					case EAGAIN:
						Logger::logMsg(ERROR, "Accept would block, retrying...");
						break;
					case ECONNABORTED:
						Logger::logMsg(ERROR, "ECONNABORTED error");
						break;
					case ENFILE:
						Logger::logMsg(ERROR, "ENFILE error");
						break;
					default:
						Logger::logMsg(ERROR, "Accept error");
						break;
				}
			}
			else
			{
				client_fds.push_back(new_socket);
				std::cout << "New client fd: " << new_socket << std::endl;
				client_fd_to_port[new_socket] = server_fd_to_port[server_fd];
				// FD_SET(new_socket, &read_fds);
				FD_SET(new_socket, &copy_read_fds);
				// FD_SET(new_socket, &write_fds);
				FD_SET(new_socket, &copy_write_fds);
				if (new_socket > max_fd)
				{
					max_fd = new_socket;
				}
				Logger::logMsg(INFO, "New connection on server port %d, client port %d, socket fd: %d", server_fd_to_port[server_fd], ntohs(client_address.sin_port), new_socket);
			}
		}
	}
}

void	Server::handleClientData(std::vector<int>& client_fds, std::map<int, int>& client_fd_to_port, char* buffer)
{
	for (size_t i = 0; i < client_fds.size();)
	{
		std::cout << "Client fd: " << client_fds[i] << std::endl;
		int client_fd = client_fds[i];
		if (FD_ISSET(client_fd, &copy_read_fds))
		{
			std::cout << "Client fd is set" << std::endl;
			int valread = read(client_fd, buffer, BUFF_SIZE);
			if (valread <= 0)
			{
				Logger::logMsg(INFO, "Connection closed by client");
				close(client_fd);
				FD_CLR(client_fd, &read_fds);  // Remove from master read_fds
				client_fds.erase(client_fds.begin() + i);
				client_fd_to_port.erase(client_fd);
				client_buffers.erase(client_fd);
			}
			else
			{
				client_buffers[client_fd].append(buffer, valread);
				if (isRequestComplete(client_buffers[client_fd]))
				{
					processRequest(client_fd, client_fd_to_port);
					handleWritableClientSockets(client_fds, client_fd_to_port);
					++i; // Only increment i if the client is not erased
				}
				else
				{
					Logger::logMsg(ERROR, "Request is not complete");
					++i; // Increment i if the request is not complete
				}
			}
		}
		else
		{
			++i; // Increment i if FD_ISSET is false
		}
	}
}

void	Server::processRequest(int client_fd, const std::map<int, int>& client_fd_to_port)
{
	int port = client_fd_to_port.at(client_fd);
	int matching_config = -1;

	for (size_t j = 0; j < configs.size(); j++)
	{
		if (configs[j].getPort() == port)
		{ 
			matching_config = j;
		}
	}

	if (matching_config == -1) {
		Logger::logMsg(ERROR, "No matching config found for port %d", port);
		return;
	}

	Request request(configs[matching_config]);
	request.parseRequest(client_buffers[client_fd].c_str());
	RequestHandler handler(client_fd, request, configs[matching_config]);
	handler.handleRequest();
	response_to_client = handler.getResponse();
	client_buffers[client_fd].clear();
}

void	Server::handleWritableClientSockets(std::vector<int>& client_fds, std::map<int, int>& client_fd_to_port)
{
	(void)client_fd_to_port;
	static std::map<int, size_t> bytes_sent;

	for (size_t i = 0; i < client_fds.size();)
	{
		int client_fd = client_fds[i];
		if (FD_ISSET(client_fd, &copy_write_fds))
		{
			size_t total_length = response_to_client.length();

			// std::cout << "response_to_client: " << response_to_client << std::endl;

			if (bytes_sent.find(client_fd) == bytes_sent.end())
			{
				bytes_sent[client_fd] = 0;
			}

			size_t remaining_data = total_length - bytes_sent[client_fd];
			const char* data_to_send = response_to_client.c_str() + bytes_sent[client_fd];

			ssize_t sent = send(client_fd, data_to_send, remaining_data, 0);
			if (sent < 0)
			{
				Logger::logMsg(ERROR, "Send error");
				close(client_fd);
				FD_CLR(client_fd, &read_fds); // Remove from master read_fds
				FD_CLR(client_fd, &write_fds); // Remove from master write_fds
				client_fds.erase(client_fds.begin() + i);
				bytes_sent.erase(client_fd);
				client_fd_to_port.erase(client_fd);
			}
			else
			{
				bytes_sent[client_fd] += sent;

				std::cout << "bytes_sent: " << bytes_sent[client_fd] << std::endl;
				std::cout << "total_length: " << total_length << std::endl;

				if (bytes_sent[client_fd] == total_length)
				{
					bytes_sent.erase(client_fd);
					close(client_fd);
					Logger::logMsg(INFO, "Connection client_fd %d closed", client_fd);
					FD_CLR(client_fd, &read_fds); // Remove from master read_fds
					FD_CLR(client_fd, &write_fds); // Remove from master write_fds
					client_fds.erase(client_fds.begin() + i);
					Logger::logMsg(INFO, "Cient_fds %d erased", client_fd);
					Logger::logMsg(INFO, "Response sent to client");
					response_to_client.clear();
				}
				else
				{
					++i; // Increment i if the client is not erased
				}
			}
		}
		else
		{
			++i; // Increment i if FD_ISSET is false
		}
	}
}

void Server::run()
{
	char buffer[BUFF_SIZE] = {0};
	int activity;
	int max_fd;
	std::vector<int> client_fds;
	std::map<int, int> client_fd_to_port;
	matching_config = -1;

	max_fd = 0;

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);

	while (true)
	{
		FD_ZERO(&copy_read_fds);
		FD_ZERO(&copy_write_fds);

		addServerSocketsToReadFds(max_fd);
		addClientSocketsToReadFds(client_fds, max_fd);
		addClientSocketsToWriteFds(client_fds, max_fd);

		copy_read_fds = read_fds;
		copy_write_fds = write_fds;

		activity = select(max_fd + 1, &copy_read_fds, &copy_write_fds, NULL, NULL);
		if (activity < 0 && errno != EINTR)
		{
			Logger::logMsg(ERROR, "Select error");
			continue;
		}

		handleNewConnections(client_fds, client_fd_to_port, max_fd);
		handleClientData(client_fds, client_fd_to_port, buffer);
		// handleWritableClientSockets(client_fds, client_fd_to_port);
		std::cout << "==== END OF LOOP ====" << std::endl;
	}
}

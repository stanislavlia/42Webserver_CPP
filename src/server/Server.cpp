#include "Server.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <map>

bool    Server::isRequestComplete(const std::string& request, int client_fd) 
{
	size_t headers_end = request.find("\r\n\r\n");
	if (headers_end == std::string::npos) 
	{
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
			return false; // Chunked body is not complete
		}
		if (request.substr(chunked_end, 5) != "0\r\n\r\n") 
		{
			return false; // EOF is not at the end
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

		if (body_length < content_length) 
		{
			connection_states[client_fd] = CHUNKED;
			return false; // Body is not complete
		}
		else
		{
			connection_states[client_fd] = CHUNKED_COMPLETE;
			return true; // Request is complete
		}
	}
	connection_states[client_fd] = READING;
	return true; // Request is complete
}

void	Server::addServerSocketsToReadFds(int& max_fd) 
{
	for (size_t i = 0; i < _server_fds.size(); i++) 
	{
		FD_SET(_server_fds[i], &monitor->getReadFds());
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
			FD_SET(client_fds[i], &monitor->getReadFds());
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
			FD_SET(client_fds[i], &monitor->getWriteFds());
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
		if (FD_ISSET(server_fd, &monitor->getReadFds()))
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
				client_fd_to_port[new_socket] = server_fd_to_port[server_fd];
				client_address_map[new_socket] = ntohs(client_address.sin_port);
				connection_states[new_socket] = READING; // Initialize connection state
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
		int client_fd = client_fds[i];
		if (FD_ISSET(client_fd, &monitor->getReadFds()))
		{
			int valread = read(client_fd, buffer, BUFF_SIZE);
			monitor->incrementReadCount(client_fd);
			if (valread <= 0) 
			{
				close(client_fd);
				FD_CLR(client_fd, &monitor->getReadFds());
				client_fds.erase(client_fds.begin() + i);
				client_fd_to_port.erase(client_fd);
				client_buffers.erase(client_fd);
				client_address_map.erase(client_fd);
				connection_states.erase(client_fd);
			} 
			else 
			{
				client_buffers[client_fd].append(buffer, valread);
				if (connection_states[client_fd] == CHUNKED || connection_states[client_fd] == CHUNKED_COMPLETE)
				{
					incrementWriteCount(client_fd);
				}
				if (isRequestComplete(client_buffers[client_fd], client_fd) == true || getWriteCount(client_fd) > 2500 || connection_states[client_fd] == CHUNKED_COMPLETE)
				{
					processRequest(client_fd, client_fd_to_port, client_fds);
					write_count[client_fd] = 0;
					if (connection_states[client_fd] == CHUNKED)
					{
						++i;
						continue ;
					}
					connection_states[client_fd] = WRITING; // Update state to WRITING
					++i;
				}
				else
				{
					++i;
				}
			}
		}
		else
		{
			++i;
		}
	}
}

void	Server::processRequest(int client_fd, const std::map<int, int>& client_fd_to_port, std::vector<int>& client_fds) 
{
	int port = client_fd_to_port.at(client_fd);
	int matching_config = -1;

	for (size_t j = 0; j < configs.size(); j++) 
	{
		if (configs[j].getPort() == port) 
		{
			matching_config = j;
			break ;
		}
	}

	if (matching_config == -1) 
	{
		Logger::logMsg(ERROR, "No matching config found for port %d", port);
		return;
	}

	Request request(configs[matching_config]);
	request.parseRequest(client_buffers[client_fd]);
	RequestHandler handler(client_fd, request, configs[matching_config], monitor);
	handler.handleRequest(client_fds, client_fd, connection_states[client_fd]);
	if (connection_states[client_fd] == CHUNKED)
	{
		return ;
	}
	response_to_client[client_fd] = handler.getResponse();
	client_buffers[client_fd].clear();
}

void	_serveHtmlContent(const std::string& html_content, std::string& response, int status_code, const std::string& status_message)
{
	std::stringstream ss;
	ss << html_content.length();


	std::stringstream status_code_ss;
	status_code_ss << status_code;

	response = "HTTP/1.1 " + status_code_ss.str() + " " + status_message + "\r\n"
						   "Content-Type: text/html\r\n"
						   "Content-Length: " + ss.str() + "\r\n"
						   "\r\n" + html_content;
}

void	Server::handleWritableClientSockets(std::vector<int>& client_fds, std::map<int, int>& client_fd_to_port) 
{
	static std::map<int, size_t> bytes_sent;
	for (size_t i = 0; i < client_fds.size();) 
	{
		int client_fd = client_fds[i];
		if (FD_ISSET(client_fd, &monitor->getWriteFds()) && connection_states[client_fd] == WRITING)
		{
			if (monitor->getCgiState(client_fd) != NO_STATE)
			{
				if (monitor->getCgiState(client_fd) == CGI_READING)
				{
					if (monitor->getReadCount(client_fd) > 0)
						return ;
					char buffer[4096];
					if (monitor->getCgiStatus(client_fd) == 0)
					{
						monitor->incrementReadCount(client_fd);
						size_t bytes_read = read(monitor->getCgiPipe(client_fd), buffer, sizeof(buffer));
						std::string html_content;
						html_content.append(buffer, bytes_read);
						_serveHtmlContent(html_content, response_to_client[client_fd], 200, "OK");
					}
				}
			}
			if (monitor->getWriteCount(client_fd) > 0)
				return ;
			monitor->incrementWriteCount(client_fd);
			size_t total_length = response_to_client[client_fd].length();

			if (bytes_sent.find(client_fd) == bytes_sent.end()) 
			{
				bytes_sent[client_fd] = 0;
			}

			size_t remaining_data = total_length - bytes_sent[client_fd];
			const char* data_to_send = response_to_client[client_fd].c_str() + bytes_sent[client_fd];

			ssize_t sent = send(client_fd, data_to_send, remaining_data, 0);
			if (sent <= 0) 
			{
				Logger::logMsg(ERROR, "Send error");
				close(client_fd);
				FD_CLR(client_fd, &monitor->getReadFds());
				FD_CLR(client_fd, &monitor->getWriteFds());
				client_fds.erase(client_fds.begin() + i);
				bytes_sent.erase(client_fd);
				client_fd_to_port.erase(client_fd);
				client_address_map.erase(client_fd);
				connection_states.erase(client_fd);
			} 
			else 
			{
				bytes_sent[client_fd] += sent;
				if (bytes_sent[client_fd] == total_length) 
				{
					if (monitor->getCgiState(client_fd) != NO_STATE)
					{
						close(monitor->getCgiPipe(client_fd));
						monitor->removeCgiPipe(client_fd);
						monitor->removeCgiState(client_fd);
						monitor->removeCgiStatus(client_fd);
						alarm(0);
					}
					bytes_sent.erase(client_fd);
					close(client_fd);
					FD_CLR(client_fd, &monitor->getReadFds());
					FD_CLR(client_fd, &monitor->getWriteFds());
					client_fds.erase(client_fds.begin() + i);
					response_to_client[client_fd].clear();
					client_address_map.erase(client_fd);
					connection_states.erase(client_fd);
					write_count.erase(client_fd);
					i++;
				} 
				else
				{
					++i;
				}
			}
		} 
		else 
		{
			++i;
		}
	}
}

void Server::multiplexSocket(int &max_fd, std::vector<int> &client_fds)
{
	FD_ZERO(&monitor->getReadFds());
	FD_ZERO(&monitor->getWriteFds());

	addServerSocketsToReadFds(max_fd);
	addClientSocketsToReadFds(client_fds, max_fd);
	addClientSocketsToWriteFds(client_fds, max_fd);

}

void Server::run()
{
	char buffer[BUFF_SIZE] = {0};
	int activity;
	int max_fd = 0;
	std::vector<int> client_fds;
	std::map<int, int> client_fd_to_port;

	while (true)
	{
		multiplexSocket(max_fd, client_fds);

		struct timeval timeout = {5, 0};

		activity = select(max_fd + 1, &monitor->getReadFds(), &monitor->getWriteFds(), NULL, &timeout);
		if (activity < 0)
		{
			Logger::logMsg(ERROR, "Select error");
			Logger::logMsg(ERROR, "Error code: %d", errno);
			continue;
		}

		monitor->resetCounts();

		handleNewConnections(client_fds, client_fd_to_port, max_fd);
		handleClientData(client_fds, client_fd_to_port, buffer);
		handleWritableClientSockets(client_fds, client_fd_to_port);
	}
}

void	Server::incrementWriteCount(int client_fd)
{
	write_count[client_fd]++;
}

int		Server::getWriteCount(int client_fd) const
{
	std::map<int, int>::const_iterator it = write_count.find(client_fd);
	if (it != write_count.end())
	{
		return it->second;
	}
	return 0;
}
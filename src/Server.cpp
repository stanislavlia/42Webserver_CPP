#include "Server.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"
#include "sstream"
#include <sys/stat.h>
#include <dirent.h>


bool	isRequestComplete(const std::string& request)
{
	// Check for the end of headers marker
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
		// std::cout << "Body length: " << body_length << std::endl;
		// std::cout << "Content length: " << content_length << std::endl;
		if (body_length < content_length)
		{
			return false; // Body is not complete
		}
	}
	return true; // Request is complete
}

void Server::addServerSocketsToReadFds(int& max_fd)
{
    for (size_t i = 0; i < _server_fds.size(); i++)
    {
        FD_SET(_server_fds[i], &read_fds);
        if (_server_fds[i] > max_fd)
        {
            max_fd = _server_fds[i];
        }
    }
}

void Server::addClientSocketsToReadFds(const std::vector<int>& client_fds, int& max_fd)
{
    for (size_t i = 0; i < client_fds.size(); i++)
    {
        if (client_fds[i] > 0)
        {
            FD_SET(client_fds[i], &read_fds);
        }
        if (client_fds[i] > max_fd)
        {
            max_fd = client_fds[i];
        }
    }
}

void Server::handleNewConnections(std::vector<int>& client_fds, std::map<int, int>& client_fd_to_port)
{
    for (size_t i = 0; i < _server_fds.size(); i++)
    {
        int server_fd = _server_fds[i];
        if (FD_ISSET(server_fd, &read_fds))
        {
            struct sockaddr_in client_address;
            int addrlen = sizeof(client_address);
            int new_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*)&addrlen);
            if (new_socket < 0)
            {
                Logger::logMsg(ERROR, "Accept error");
            }
            else
            {
                client_fds.push_back(new_socket);
                client_fd_to_port[new_socket] = server_fd_to_port[server_fd];
				Logger::logMsg(INFO, "New connection on server port %d, client port %d, socket fd: %d", server_fd_to_port[server_fd], ntohs(client_address.sin_port), new_socket);
            }
        }
    }
}

void Server::handleClientData(std::vector<int>& client_fds, std::map<int, int>& client_fd_to_port, char* buffer)
{
    for (size_t i = 0; i < client_fds.size(); )
    {
        if (FD_ISSET(client_fds[i], &read_fds))
        {
            int valread = read(client_fds[i], buffer, BUFF_SIZE);
            if (valread <= 0)
            {
                Logger::logMsg(INFO, "Connection closed by client");
                close(client_fds[i]);
                client_fds.erase(client_fds.begin() + i);
            }
            else
            {
                client_buffers[client_fds[i]].append(buffer, valread);
                if (isRequestComplete(client_buffers[client_fds[i]]))
                {
                    processRequest(client_fds[i], client_fd_to_port);
                    ++i; // Only increment i if the client is not erased
                }
            }
        }
        else
        {
            ++i; // Increment i if FD_ISSET is false
        }
    }
}

void Server::processRequest(int client_fd, const std::map<int, int>& client_fd_to_port)
{
	int matching_config = -1;
    int port = client_fd_to_port.at(client_fd);

	for (size_t j = 0; j < configs.size(); j++)
	{
		if (configs[j].getPort() == port)
		{
			matching_config = j;
		}
	}

	if (matching_config == -1)
	{
		Logger::logMsg(ERROR, "No matching config found for port %d", port);
		return;
	}

	Request request(configs[matching_config]);
	request.parseRequest(client_buffers[client_fd].c_str());
	RequestHandler handler(client_fd, request, configs[matching_config]);
	handler.handleRequest();
	client_buffers[client_fd].clear();
}


void Server::run()
{
    char buffer[BUFF_SIZE] = {0};
    int activity;
    int max_fd;
    std::vector<int> client_fds;
    std::map<int, int> client_fd_to_port;

    while (true)
    {
        FD_ZERO(&read_fds);
        max_fd = 0;

        // Add server sockets to read_fds
        addServerSocketsToReadFds(max_fd);

        // Add client sockets to read_fds
        addClientSocketsToReadFds(client_fds, max_fd);

        // Wait for activity on one of the sockets
        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0)
        {
            Logger::logMsg(ERROR, "Select error");
            continue;
        }

        // Handle new connections
        handleNewConnections(client_fds, client_fd_to_port);

        // Handle incoming data on client sockets
        handleClientData(client_fds, client_fd_to_port, buffer);
    }
}

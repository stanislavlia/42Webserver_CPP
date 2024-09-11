#include "Server.hpp"
#include "Request.hpp"
#include "sstream"


Server::Server()
{
};

Server::Server(struct sockaddr_in  *sock_address, const std::vector<ServerParam>& server_param) 
: configs(server_param) , _sock_address(sock_address)
{
	Logger::logMsg(DEBUG, "Server initialized");
	_port = server_param[0].getPort();
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
		Logger::logMsg(ERROR, "Socket creation failed");
		exit(EXIT_FAILURE); 
	};
	// Set the server socket to non-blocking mode
	fcntl(_server_fd, F_SETFL, O_NONBLOCK);
};


void Server::_set_socket_options(int opt)
{
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0)
	{
		Logger::logMsg(ERROR, "setsockopt failed"); //replace with exceptions later
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
		Logger::logMsg(ERROR, "Failed to bind socket");
		close(_server_fd);
		exit(EXIT_FAILURE);
	}
};

void    Server::_listen_socket()
{
	if (listen(_server_fd, CONN_QUEUE) < 0)
	{
		Logger::logMsg(ERROR, "Failed to listen");
		close(_server_fd);
		exit(EXIT_FAILURE);
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


std::string Server::render_html(const std::string& path)
{
    std::ifstream file(path.c_str());

	std::cout << "PATH: " << path << std::endl;

    if (!file.is_open())
    {
        Logger::logMsg(ERROR, "Failed to open HTML file: %s", path.c_str());
        return render_html("./static/not_found.html");
    }

    std::stringstream stream_buffer;
    stream_buffer << file.rdbuf();
    return stream_buffer.str();
}

void Server::respond_with_error(int socket, int status_code, const std::string& status_message)
{
	std::string path = configs[0].getErrorPage().at(status_code);

	std::string html_content = render_html(path);
	std::stringstream ss;
    ss << html_content.length();

    std::stringstream status_code_ss;
    status_code_ss << status_code;

    std::string response = "HTTP/1.1 " + status_code_ss.str() + " " + status_message + "\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: " + ss.str() + "\r\n"
                           "\r\n" + html_content;

    send(socket, response.c_str(), response.length(), 0);
}


void Server::respond_with_html(int socket, const std::string& path, int status_code, const std::string& status_message)
{
    std::string html_content = render_html(path); //need to add exceptions

    std::stringstream ss;
    ss << html_content.length();

    std::stringstream status_code_ss;
    status_code_ss << status_code;

    std::string response = "HTTP/1.1 " + status_code_ss.str() + " " + status_message + "\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: " + ss.str() + "\r\n"
                           "\r\n" + html_content;

    send(socket, response.c_str(), response.length(), 0);
}

// void	Server::respond_with_html(int client_fd, const std::string& path)
// {
// 	std::string html_content = render_html(path); //need to add exceptions

// 	std::stringstream ss;
// 	ss << html_content.length();

// 	std::string response = "HTTP/1.1 200 OK\r\n"
// 						"Content-Type: text/html\r\n"
// 						"Content-Length: " + ss.str() + "\r\n"
// 						"\r\n" + html_content;

// 	send(client_fd, response.c_str(), response.length(), 0);
// 	std::cout << "RESPONSE: " << response << std::endl;
// };



void    Server::setup_server()
{
	_create_server_socket();
	_set_socket_options(1);
	_setup_socketaddress(INADDR_ANY); // 0.0.0.0 - special address that listen on all available net interfaces
	
	FD_ZERO(&read_fds);
	FD_SET(_server_fd, &read_fds);

	Logger::logMsg(INFO, "Server socket created and configured successfully");
	Logger::logMsg(DEBUG, "Server FD: %d", _server_fd);

	_bind_socket();
	_listen_socket();
	Logger::logMsg(DEBUG, "Listening on port: %d", _port);

};

void Server::run()
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
				// std::cout << "BUFFER: " << buffer << std::endl;
				valread = read(i, buffer, sizeof(buffer));
				if (valread == 0)
				{
					Logger::logMsg(INFO, "Client disconnected; SOCKET FD: %d", i);
					close(i);
					FD_CLR(i, &read_fds);
				}
				else if (valread > 0)
				{
					Request	request(configs[0]);
					request.parseRequest(buffer);

					if (request.isValid() != 0)
					{
						if (request.isValid() == 1 || request.isValid() == 3)
						{
							// 400 Bad Request
							respond_with_error(i, 400, "Bad Request");
							Logger::logMsg(ERROR, "Bad Request %d - code", 400);
						}
						if (request.isValid() == 2)
						{
							// 405 Method Not Allowed	
							respond_with_error(i, 405, "Method Not Allowed");
							Logger::logMsg(ERROR, "Method Not Allowed %d - code", 405);
						}
					}
					else
					{
						if (request.getMethod() == "GET" && request.getUri() == "/")
							respond_with_html(i, "./static/index.html", 200, "OK");
						else if (request.getMethod() == "GET" && request.getUri() == "/home")
							respond_with_html(i, "./static/home.html", 200, "OK");
						else if (strncmp(buffer, "GET ", 4) == 0)
						{
							respond_with_html(i, "./static/not_found.html", 404, "Not Found");
							Logger::logMsg(ERROR, "No page FOUND %d - code", 404);
						}
					}
				}
			}   
		}
	}
};

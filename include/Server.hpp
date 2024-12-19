#ifndef SERVER_HPP
# define SERVER_HPP

#include "Webserver.hpp"
#include "Logger.hpp"
#include "ServerParam.hpp"

class Server
{  
	private:
		std::string _complete_request;
		std::vector<ServerParam> configs;
		std::map<int, std::string> client_buffers;
		// std::map<int, bool> client_requests_complete;]
		// int _port;
		std::vector<int>	_ports;
		std::map<int, int>server_fd_to_port;
		// const char* _host;
		std::string _host;
		// struct sockaddr_in  *_sock_address;
		std::vector<struct sockaddr_in> _sock_address;
		// int  _server_fd;
		std::vector<int>	_server_fds;
		fd_set  read_fds;
		fd_set  write_fds;
		fd_set  copy_read_fds;
		fd_set  copy_write_fds;
		const std::string _server_name;
		int	matching_config;
		std::string response_to_client;

		// void    _create_server_socket();
		int		_create_server_socket();
		// void    _set_socket_options(int opt);
		void	_set_socket_options(int opt, int server_fd);
		// void    _setup_socketaddress();
		void	_setup_socketaddress(int port, int server_fd);
		// void    _bind_socket();
		// void	_bind_socket(int server_fd);
		// void     _listen_socket();
		void	_listen_socket(int server_fd);
		// int     _accept_connection();
		int		_accept_connection(int server_fd);

		// =======================================

		void  	addServerSocketsToReadFds(int& max_fd);
		void  	addClientSocketsToReadFds(const std::vector<int>& client_fds, int& max_fd);
		void	addClientSocketsToWriteFds(const std::vector<int>& client_fds, int& max_fd);
		void  	handleNewConnections(std::vector<int>& client_fds, std::map<int, int>& client_fd_to_port, int& max_fd);
		void  	handleClientData(std::vector<int>& client_fds, std::map<int, int>& client_fd_to_port, char* buffer);
		void	processRequest(int client_fd, const std::map<int, int>& client_fd_to_port);
		void	handleWritableClientSockets(std::vector<int>& client_fds, std::map<int, int>& client_fd_to_port);

		// std::string render_html(const std::string& path);
		// void        respond_with_html(int client_fd, const std::string& path, int status_code, const std::string& status_message);
		// void        respond_with_error(int socket, int status_code, const std::string& status_message);
		// void		handleFileOrDirectoryRequest(int client_fd, const std::string& full_path, const std::string& uri);
		// void		handleSpecificUriRequest(int client_fd, const std::string& rootDir, const std::string& uri);
		// void		handleRootDirectoryRequest(int client_fd, const std::string& rootDir, const std::string& uri);
		// void		handleInvalidRequest(int client_fd, int validation_code);
		
	
	public:
		// Server(struct sockaddr_in  *_sock_address, const std::vector<ServerParam>& server_param);
		Server(const std::vector<ServerParam>& server_param);
		Server();
		~Server();

		void    setup_server();
		void    run();

        //================SERVER EXCEPTIONS=====================
        class SocketHandlingException : public std::runtime_error
        {
            public:
                SocketHandlingException(const std::string& msg)
                : std::runtime_error("Socket error: " + msg) {}
        };

        class FileReadException : public std::runtime_error
        {
            public:
                FileReadException(const std::string& filename)
                : std::runtime_error("File Read Error:  " + filename) {}
        };
};

#endif
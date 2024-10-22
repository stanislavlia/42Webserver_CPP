#ifndef SERVER_HPP
# define SERVER_HPP

#include "Webserver.hpp"
#include "Logger.hpp"
#include "ServerParam.hpp"

class Server
{  
	private:
		std::string _request_data;
		std::vector<ServerParam> configs;
		int _port;
		// const char* _host;
		std::string _host;
		struct sockaddr_in  *_sock_address;
		int  _server_fd;
		fd_set  read_fds;
		const std::string _server_name; 

		void    _create_server_socket();
		void    _set_socket_options(int opt);
		void    _setup_socketaddress();
		void    _bind_socket();
		void     _listen_socket();
		int     _accept_connection();

		// std::string render_html(const std::string& path);
		// void        respond_with_html(int client_fd, const std::string& path, int status_code, const std::string& status_message);
		// void        respond_with_error(int socket, int status_code, const std::string& status_message);
		// void		handleFileOrDirectoryRequest(int client_fd, const std::string& full_path, const std::string& uri);
		// void		handleSpecificUriRequest(int client_fd, const std::string& rootDir, const std::string& uri);
		// void		handleRootDirectoryRequest(int client_fd, const std::string& rootDir, const std::string& uri);
		// void		handleInvalidRequest(int client_fd, int validation_code);
		
	
	public:
		Server(struct sockaddr_in  *_sock_address, const std::vector<ServerParam>& server_param);
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
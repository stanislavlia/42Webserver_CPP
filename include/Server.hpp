#ifndef SERVER_HPP
# define SERVER_HPP

#include "Webserver.hpp"
#include "Logger.hpp"
#include "ServerParam.hpp"

class Server
{  
	private:
		std::vector<ServerParam> configs;
		int _port;
		struct sockaddr_in  *_sock_address;
		int  _server_fd;
		fd_set  read_fds;
		const std::string _server_name; 

		void    _create_server_socket();
		void    _set_socket_options(int opt);
		void    _setup_socketaddress(int host);
		void    _bind_socket();
		void     _listen_socket();
		int     _accept_connection();

		std::string render_html(const std::string& path);
		void        respond_with_html(int client_fd, const std::string& path, int status_code, const std::string& status_message);
		
	
	public:
		Server(struct sockaddr_in  *_sock_address, const std::vector<ServerParam>& server_param);
		Server();
		~Server();

		void    setup_server();
		void    run();

	
};

#endif
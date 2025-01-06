#ifndef SERVER_HPP
# define SERVER_HPP

#include "Webserver.hpp"
#include "Logger.hpp"
#include "ServerParam.hpp"
#include "Monitor.hpp"
#include <sys/wait.h>
#include <signal.h>

// enum ConnectionState { READ, WRITE };
enum ConnectionState {READING, WRITING, CGI};

class Server
{  
	private:
		Monitor							*monitor;
		int								_reading_count;
		int								_writing_count;
		std::string						_complete_request;
		std::vector<ServerParam>		configs;
		std::map<int, std::string> 		client_buffers;
		std::vector<int>				_ports;
		std::map<int, int>				server_fd_to_port;
		std::map<int, int>				client_address_map;
		std::map<int, ConnectionState>	connection_states;
		std::string						_host;
		std::vector<struct sockaddr_in> _sock_address;
		std::vector<int>				_server_fds;
		const std::string				_server_name;
		int								matching_config;
		// std::string						response_to_client;
		std::map<int, std::string>		response_to_client;

		int		_create_server_socket();
		void	_set_socket_options(int opt, int server_fd);
		void	_setup_socketaddress(int port, int server_fd);
		void	_listen_socket(int server_fd);

		// =======================================

		void	multiplexSocket(int &max_fd, std::vector<int> &client_fds);
		// void	multiplexSocket(fd_set &copy_read_fds, fd_set &copy_write_fds, int &max_fd, std::vector<int> &client_fds);
		void  	addServerSocketsToReadFds(int& max_fd);
		void  	addClientSocketsToReadFds(const std::vector<int>& client_fds, int& max_fd);
		void	addClientSocketsToWriteFds(const std::vector<int>& client_fds, int& max_fd);
		void  	handleNewConnections(std::vector<int>& client_fds, std::map<int, int>& client_fd_to_port, int& max_fd);
		void  	handleClientData(std::vector<int>& client_fds, std::map<int, int>& client_fd_to_port, char* buffer);
		void	processRequest(int client_fd, const std::map<int, int>& client_fd_to_port, std::vector<int>& client_fds);
		void	handleWritableClientSockets(std::vector<int>& client_fds, std::map<int, int>& client_fd_to_port);
		
		int 	_waitForChildProcess(pid_t pid);

		// ======================================= TEST MONITOR FUNCTIONS


	public:
		// Server(struct sockaddr_in  *_sock_address, const std::vector<ServerParam>& server_param);
		// Server(const std::vector<ServerParam>& server_param);
		// Server();
		// ~Server();
		Server(Monitor* mon, const std::vector<ServerParam>& server_param);
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
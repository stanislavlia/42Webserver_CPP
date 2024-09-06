#ifndef SERVER_HPP
# define SERVER_HPP

#include "Webserv.hpp"

class Server
{  
    private:
        int _port;
        struct sockaddr_in  *_sock_address;
        int  _server_fd;
        fd_set  read_fds;

        int  _create_server_socket();
        void    _set_socket_options(int opt);
        void    _setup_socketaddress();
        void    _bind_socket();
        int     _listen_socket();
        int     _accept_connection();

    
    public:
        Server(struct sockaddr_in  *_sock_address, int port);
        Server();
        ~Server();

        void    setup_server();
        void    run();

    
};

#endif
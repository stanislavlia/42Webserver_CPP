#include "Webserver.hpp"

#define PORT 8012
#define BUFF_SIZE 1024
#define CONN_QUEUE 4

int create_server_fd()
{
    int server_fd;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        std::cerr << "Socket creation failed\n";
        exit(EXIT_FAILURE); 
    }
    return server_fd;
};


void set_socket_options(int server_fd, int opt)
{
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0)
    {
        std::cerr << "setsockopt failed\n";
        close(server_fd);
        exit(EXIT_FAILURE);
    }
}


void setup_socketaddress(struct sockaddr_in *sock_address, int host, int port)
{
    sock_address->sin_family = AF_INET;
    sock_address->sin_addr.s_addr = host;
    sock_address->sin_port = htons(port);
};

void bind_socket(int server_fd, struct sockaddr_in *sock_address)
{
    if (bind(server_fd, (struct sockaddr *)sock_address, sizeof(*sock_address)) < 0)
    {
        std::cerr << "Failed to bind socket" << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }
}

void listen_socket(int server_fd)
{
    if (listen(server_fd, CONN_QUEUE) < 0)
    {
        std::cerr << "Failed to listen\n";
        close(server_fd);
        exit(EXIT_FAILURE);
    }
}

int accept_connection_socket(int server_fd, struct sockaddr_in *sock_address)
{
    int new_socket;
    socklen_t addrlen = sizeof(*sock_address);

    new_socket = accept(server_fd, (struct sockaddr*) sock_address, &addrlen);

    if (new_socket < 0)
    {
        std::cerr << "Accept failed." << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    return new_socket;
};


//CLI to check listening sockets: lsof -i -P -n | grep LISTEN
//CLI to test echo-serv: telnet localhost 8012
//CLI to test echo-serv: nc localhost 8012


int main()
{
    int server_fd = create_server_fd();
    int opt = 1;
    int new_socket;
    int valread;
    char buffer[BUFF_SIZE] = {0};
    
    set_socket_options(server_fd, opt);

    struct sockaddr_in address;
    setup_socketaddress(&address, INADDR_ANY, PORT);

    std::cout << "Server socket created and configured successfully.\n";
    std::cout << "Server FD: " << server_fd << std::endl;

    // Bind, listen, and accept would follow here in a full implementation.
    bind_socket(server_fd, &address);
    listen_socket(server_fd);
    std::cout << "Listening on on port: " << PORT << std::endl; 
    
    while (true)
    {
        new_socket = accept_connection_socket(server_fd, &address);
        std::cout << "Connection accepted, new socket_fd: " << new_socket << std::endl;
        
        // Reset buffer for new connection
        memset(buffer, 0, BUFF_SIZE);

        valread = read(new_socket, buffer, BUFF_SIZE);
        if (valread > 0)
        {
            std::cout << "Received " << valread << " bytes" << std::endl;
            std::cout << "Content: " << buffer << std::endl;

            // Send back the same message
            send(new_socket,  "[SERV_ECHO]: ", 14, 0);
            send(new_socket, buffer, valread, 0);
            
            std::cout << "Echoed message back to client." << std::endl;
        }
        else if (valread == 0)
        {
            std::cout << "Client disconnected." << std::endl;
        }
        else
        {
            std::cerr << "Read error." << std::endl;
        }

        close(new_socket);
    }
    
    close(server_fd);
    return 0;
    return 0;
}

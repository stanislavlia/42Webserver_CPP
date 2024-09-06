// #include "Webserver.hpp"
// #include <fcntl.h>  // Needed for fcntl()


// int create_server_fd()
// {
//     int server_fd;

//     server_fd = socket(AF_INET, SOCK_STREAM, 0);

//     if (server_fd == -1)
//     {
//         std::cerr << "Socket creation failed\n";
//         exit(EXIT_FAILURE); 
//     }

//     // Set the server socket to non-blocking mode
//     fcntl(server_fd, F_SETFL, O_NONBLOCK);

//     return server_fd;
// }

// void set_socket_options(int server_fd, int opt)
// {
//     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0)
//     {
//         std::cerr << "setsockopt failed\n";
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }
// }

// void setup_socketaddress(struct sockaddr_in *sock_address, int host, int port)
// {
//     sock_address->sin_family = AF_INET;
//     sock_address->sin_addr.s_addr = host;
//     sock_address->sin_port = htons(port);
// }

// void bind_socket(int server_fd, struct sockaddr_in *sock_address)
// {
//     if (bind(server_fd, (struct sockaddr *)sock_address, sizeof(*sock_address)) < 0)
//     {
//         std::cerr << "Failed to bind socket" << std::endl;
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }
// }

// void listen_socket(int server_fd)
// {
//     if (listen(server_fd, CONN_QUEUE) < 0)
//     {
//         std::cerr << "Failed to listen\n";
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }
// }

// int accept_connection_socket(int server_fd, struct sockaddr_in *sock_address)
// {
//     int new_socket;
//     socklen_t addrlen = sizeof(*sock_address);

//     new_socket = accept(server_fd, (struct sockaddr*) sock_address, &addrlen);

//     if (new_socket < 0)
//     {
//         if (errno != EWOULDBLOCK)  // Accepting failed for reasons other than non-blocking
//         {
//             std::cerr << "Accept failed." << std::endl;
//             close(server_fd);
//             exit(EXIT_FAILURE);
//         }
//     }
//     else
//     {
//         // Set the new socket to non-blocking mode
//         fcntl(new_socket, F_SETFL, O_NONBLOCK);
//     }
//     return new_socket;
// }

// int main()
// {
//     int server_fd = create_server_fd();
//     int opt = 1;
//     int new_socket;
//     int valread;
//     char buffer[BUFF_SIZE] = {0};
//     struct sockaddr_in address;
     
//     fd_set readfds; //set of FDs for I/O ops
//     int max_fd, activity;
    
//     set_socket_options(server_fd, opt);

    
//     setup_socketaddress(&address, INADDR_ANY, PORT);

//     FD_ZERO(&readfds);
//     FD_SET(server_fd, &readfds);
//     max_fd = server_fd;

//     std::cout << "Server socket created and configured successfully.\n";
//     std::cout << "Server FD: " << server_fd << std::endl;

//     bind_socket(server_fd, &address);
//     listen_socket(server_fd);
//     std::cout << "Listening on port: " << PORT << std::endl; 
    
//     while (true)
//     {
//         fd_set current_fds = readfds;

//         //================MULTIPLEXING=================
//         activity = select(max_fd + 1, &current_fds, NULL, NULL, NULL);
//         if (activity < 0)
//         {
//             std::cerr << "select() failed\n" << std::endl;
//             continue;
//         }

//         // Check if it's an incoming connection
//         if (FD_ISSET(server_fd, &current_fds))
//         {
//             new_socket = accept_connection_socket(server_fd, &address);
//             if (new_socket > 0)
//             {
//                 std::cout << "Connection accepted; fd: " << new_socket << std::endl;

//                 // Add new socket to pool of fds
//                 FD_SET(new_socket, &readfds);
//                 if (new_socket > max_fd)
//                     max_fd = new_socket;
//             }
//         }

//         //============PERFORM I/O OPERATIONS on other sockets from Pool========
//         for (int i = 0; i <= max_fd; i++)
//         {
//             if (FD_ISSET(i, &current_fds))
//             {
//                 memset(buffer, 0, BUFF_SIZE);
//                 valread = read(i, buffer, BUFF_SIZE);

//                 if (valread > 0)
//                 {
//                     std::cout << "Received " << valread << " bytes on socket_fd: " << i << std::endl;
//                     std::cout << "Content: " << buffer << std::endl;

//                     // Send back the same message with prefix
//                     send(i, "[SERV_ECHO]: ", 14, 0);
//                     send(i, buffer, valread, 0);
                    
//                     std::cout << "Echoed message back to client on socket_fd: " << i << std::endl;
//                 }
//                 else if (valread == 0)
//                 {
//                     // Client disconnected
//                     std::cout << "Client disconnected from socket_fd: " << i << std::endl;
//                     close(i);
//                     FD_CLR(i, &readfds);
//                 }
//                 else if (valread < 0 && errno != EWOULDBLOCK)
//                 {
//                     std::cerr << "Read error on socket_fd: " << i << std::endl;
//                     close(i);
//                     FD_CLR(i, &readfds);
//                 }
//             }
//         }
//     }

//     close(server_fd);
//     return 0;
// }

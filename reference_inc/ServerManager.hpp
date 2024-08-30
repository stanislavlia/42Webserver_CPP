#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

#include "Webserv.hpp"
#include "Client.hpp"
#include "Response.hpp"

/**
 * @class ServerManager
 * @brief Manages the operation of the webserver, including server setup, client connections, and request/response handling.
 *
 * The ServerManager class is responsible for running the webserver based on configurations extracted from the configuration file.
 * It handles the setup and operation of servers, manages client connections, and processes incoming requests and outgoing responses.
 * The class maintains file descriptor sets for reading and writing, ensuring non-blocking I/O operations for multiple clients.
 *
 * Attributes:
 *  - _servers: A vector of server configurations for the servers to be run by the webserver.
 *  - _servers_map: A map associating server socket file descriptors with their corresponding server configurations.
 *  - _clients_map: A map associating client socket file descriptors with their corresponding client objects.
 *  - _recv_fd_pool: A set of file descriptors monitored for incoming data (read operations).
 *  - _write_fd_pool: A set of file descriptors monitored for outgoing data (write operations).
 *  - _biggest_fd: The largest file descriptor value, used for managing the file descriptor sets.
 */
class ServerManager
{
    public:
        // Constructors and Destructor
        ServerManager();                                 ///< Default constructor.
        ~ServerManager();                                ///< Destructor.

        // Public methods
        void setupServers(std::vector<ServerConfig>);    ///< Set up servers based on the provided configurations.
        void runServers();                               ///< Run the servers, handling client connections and I/O.

    private:
        std::vector<ServerConfig> _servers;              ///< Vector of server configurations.
        std::map<int, ServerConfig> _servers_map;        ///< Map of server socket file descriptors to server configurations.
        std::map<int, Client> _clients_map;              ///< Map of client socket file descriptors to client objects.
        fd_set _recv_fd_pool;                            ///< Set of file descriptors monitored for read operations.
        fd_set _write_fd_pool;                           ///< Set of file descriptors monitored for write operations.
        int _biggest_fd;                                 ///< Largest file descriptor, used in fd_set management.

        // Private methods
        void acceptNewConnection(ServerConfig &);        ///< Accept a new client connection on a server.
        void checkTimeout();                             ///< Check for and handle client connection timeouts.
        void initializeSets();                           ///< Initialize the file descriptor sets.
        void readRequest(const int &, Client &);         ///< Read an incoming request from a client.
        void handleReqBody(Client &);                    ///< Handle the body of a client's request.
        void sendResponse(const int &, Client &);        ///< Send an HTTP response to a client.
        void sendCgiBody(Client &, CgiHandler &);        ///< Send the CGI response body to the client.
        void readCgiResponse(Client &, CgiHandler &);    ///< Read the response from a CGI script.
        void closeConnection(const int);                 ///< Close a client connection.
        void assignServer(Client &);                     ///< Assign a server configuration to a client based on the request.
        void addToSet(const int, fd_set &);              ///< Add a file descriptor to a specified fd_set.
        void removeFromSet(const int, fd_set &);         ///< Remove a file descriptor from a specified fd_set.
};

#endif // SERVERMANAGER_HPP

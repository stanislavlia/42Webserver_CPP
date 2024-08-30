#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "Webserv.hpp"
#include "HttpRequest.hpp"
#include "Response.hpp"

/**
 * @class Client
 * @brief Manages the client's connection, storing all relevant information such as the client's socket, address, request, and response objects.
 *
 * The Client class is responsible for handling a client's connection within the web server.
 * It stores information about the client's socket, address, and the time of the last message received.
 * Additionally, it manages the HTTP request from the client and prepares an HTTP response to be sent back.
 * Each Client instance is associated with a specific server configuration.
 *
 * Attributes:
 *  - _client_socket: The socket file descriptor associated with the client's connection.
 *  - _client_address: The client's address information (IP address and port).
 *  - _last_msg_time: The time when the last message was received from the client.
 *
 * Public attributes:
 *  - request: An instance of HttpRequest that represents the HTTP request sent by the client.
 *  - response: An instance of Response that represents the HTTP response to be sent to the client.
 *  - server: An instance of ServerConfig that holds the configuration of the server the client is connected to.
 */
class Client
{
    public:
        // Constructors and Destructor
        Client();                                    ///< Default constructor.
        Client(const Client &other);                 ///< Copy constructor.
        Client(ServerConfig &serverConfig);          ///< Constructor with a server configuration parameter.
        Client &operator=(const Client & rhs);       ///< Assignment operator.
        ~Client();                                   ///< Destructor.

        // Getters
        const int                 &getSocket() const;            ///< Get the client's socket.
        const struct sockaddr_in  &getAddress() const;           ///< Get the client's address.
        const HttpRequest         &getRequest() const;           ///< Get the client's HTTP request.
        const time_t              &getLastTime() const;          ///< Get the time of the last message received from the client.

        // Setters
        void                setSocket(int &socket);              ///< Set the client's socket.
        void                setAddress(sockaddr_in &address);    ///< Set the client's address.
        void                setServer(ServerConfig &serverConfig); ///< Set the server configuration associated with the client.

        // Member functions
        void                buildResponse();                     ///< Build the HTTP response based on the request.
        void                updateTime();                        ///< Update the last message time to the current time.

        // Utility functions
        void                clearClient();                       ///< Clear the client's data and reset the client object.

        // Public attributes
        Response            response;                            ///< HTTP response object associated with the client.
        HttpRequest         request;                             ///< HTTP request object associated with the client.
        ServerConfig        server;                              ///< Server configuration associated with the client.

    private:
        int                 _client_socket;                      ///< Socket file descriptor for the client's connection.
        struct sockaddr_in  _client_address;                     ///< Address information for the client.
        time_t              _last_msg_time;                      ///< Timestamp of the last message received from the client.
};

#endif // CLIENT_HPP

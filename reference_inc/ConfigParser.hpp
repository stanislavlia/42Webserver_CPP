#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "Webserv.hpp"

class ServerConfig;

/**
 * @class ConfigParser
 * @brief Parses the server configuration file and creates server configurations for the web server.
 *
 * The ConfigParser class is responsible for reading, parsing, and processing the configuration file
 * for the web server. It splits the content into individual server configurations, removes comments and
 * unnecessary whitespace, and checks the validity of the parsed server configurations. The class stores
 * the parsed server configurations in a vector and provides methods to retrieve these configurations.
 *
 * Attributes:
 *  - _servers: A vector of ServerConfig objects representing the parsed server configurations.
 *  - _server_config: A vector of strings storing the configuration blocks for each server.
 *  - _nb_server: The number of servers defined in the configuration file.
 */
class ConfigParser {
    private:
        std::vector<ServerConfig> _servers;          ///< Parsed server configurations.
        std::vector<std::string> _server_config;     ///< Configuration blocks for each server.
        size_t _nb_server;                           ///< Number of servers defined in the configuration file.

    public:
        // Constructors and Destructor
        ConfigParser();                              ///< Default constructor.
        ~ConfigParser();                             ///< Destructor.

        // Member functions
        int createCluster(const std::string &config_file); ///< Parse the configuration file and create server configurations.

        void splitServers(std::string &content);     ///< Split the content into individual server configurations.
        void removeComments(std::string &content);   ///< Remove comments from the configuration content.
        void removeWhiteSpace(std::string &content); ///< Remove unnecessary whitespace from the configuration content.
        size_t findStartServer(size_t start, std::string &content); ///< Find the start position of a server configuration block.
        size_t findEndServer(size_t start, std::string &content);   ///< Find the end position of a server configuration block.
        void createServer(std::string &config, ServerConfig &server); ///< Create a ServerConfig object from a configuration block.
        void checkServers();                          ///< Validate the parsed server configurations.
        std::vector<ServerConfig> getServers();       ///< Retrieve the parsed server configurations.
        int stringCompare(std::string str1, std::string str2, size_t pos); ///< Compare two strings starting from a specific position.

        int print();                                  ///< Print the parsed configuration for debugging purposes.

        /**
         * @class ErrorException
         * @brief Custom exception class for handling errors during configuration parsing.
         *
         * The ErrorException class is a custom exception that is thrown when there is
         * an error during the parsing of the configuration file. It provides a detailed
         * error message indicating the nature of the error.
         */
        class ErrorException : public std::exception
        {
            private:
                std::string _message;                ///< Error message to be displayed.

            public:
                ErrorException(std::string message) throw() ///< Constructor that initializes the error message.
                {
                    _message = "CONFIG PARSER ERROR: " + message;
                }

                virtual const char* what() const throw() ///< Returns the error message.
                {
                    return (_message.c_str());
                }

                virtual ~ErrorException() throw() {} ///< Destructor.
        };
};

#endif // CONFIGPARSER_HPP

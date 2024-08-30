#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "Webserv.hpp"

class HttpRequest;

/**
 * @class CgiHandler
 * @brief Handles the execution of CGI (Common Gateway Interface) scripts in the webserver.
 *
 * The CgiHandler class is responsible for setting up the environment, executing
 * CGI scripts, and handling the communication between the webserver and the CGI
 * script. It manages the environment variables, command-line arguments, and pipes
 * needed to execute the CGI script and process its output.
 *
 * Attributes:
 *  - _env: A map of environment variables that will be passed to the CGI script.
 *  - _ch_env: A dynamically allocated array of C strings representing the environment
 *    variables in a format suitable for execve().
 *  - _argv: A dynamically allocated array of C strings representing the command-line
 *    arguments for the CGI script.
 *  - _exit_status: The exit status of the CGI script after execution.
 *  - _cgi_path: The file path to the CGI script that needs to be executed.
 *  - _cgi_pid: The process ID of the CGI script once it is forked and executed.
 *
 * Public attributes:
 *  - pipe_in: An array of two file descriptors for the pipe that handles input to the CGI script.
 *  - pipe_out: An array of two file descriptors for the pipe that handles output from the CGI script.
 */
class CgiHandler {
    private:
        std::map<std::string, std::string> _env; ///< Environment variables for the CGI script.
        char** _ch_env;                          ///< C-style array of environment variables for execve().
        char** _argv;                            ///< C-style array of arguments for the CGI script.
        int _exit_status;                        ///< Exit status of the CGI script.
        std::string _cgi_path;                   ///< Path to the CGI script.
        pid_t _cgi_pid;                          ///< Process ID of the CGI script.

    public:
        int pipe_in[2];  ///< Pipe file descriptors for input to the CGI script.
        int pipe_out[2]; ///< Pipe file descriptors for output from the CGI script.

        // Constructors and Destructor
        CgiHandler();                              ///< Default constructor.
        CgiHandler(std::string path);              ///< Constructor with a CGI path parameter.
        ~CgiHandler();                             ///< Destructor to clean up resources.
        CgiHandler(CgiHandler const &other);       ///< Copy constructor.
        CgiHandler &operator=(CgiHandler const &rhs); ///< Assignment operator.

        // Member functions
        void initEnv(HttpRequest& req, const std::vector<Location>::iterator it_loc); ///< Initialize environment variables for CGI.
        void initEnvCgi(HttpRequest& req, const std::vector<Location>::iterator it_loc); ///< Initialize additional CGI-specific environment variables.
        void execute(short &error_code);           ///< Execute the CGI script.
        void sendHeaderBody(int &pipe_out, int &fd, std::string &); ///< Send headers and body to the client.
        void fixHeader(std::string &header);       ///< Adjust HTTP headers from the CGI output.
        void clear();                              ///< Clear resources and reset the handler.
        std::string setCookie(const std::string& str); ///< Set cookies from the CGI output.

        // Setters
        void setCgiPid(pid_t cgi_pid);             ///< Set the CGI process ID.
        void setCgiPath(const std::string &cgi_path); ///< Set the CGI script path.

        // Getters
        const std::map<std::string, std::string> &getEnv() const; ///< Get the environment variables map.
        const pid_t &getCgiPid() const;            ///< Get the CGI process ID.
        const std::string &getCgiPath() const;     ///< Get the CGI script path.

        // Utility functions
        std::string getAfter(const std::string& path, char delim); ///< Get the substring after a delimiter.
        std::string getBefore(const std::string& path, char delim); ///< Get the substring before a delimiter.
        std::string getPathInfo(std::string& path, std::vector<std::string> extensions); ///< Retrieve the path info for the CGI script.
        int countCookies(const std::string& str);  ///< Count the number of cookies in a string.
        int findStart(const std::string path, const std::string delim); ///< Find the start position after a delimiter.
        std::string decode(std::string &path);     ///< URL decode the given path.
};

#endif

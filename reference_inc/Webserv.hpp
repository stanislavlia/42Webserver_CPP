#pragma once
#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
// # include <fstream>
# include <fcntl.h>
# include <cstring>
# include <string> 
# include <unistd.h>
# include <dirent.h>
# include <sstream>
// # include <bits/stdc++.h>
# include <cstdlib>
# include <fstream>
# include <sstream>
# include <cctype>
# include <ctime>
# include <cstdarg>

/* STL Containers */
# include <map>
# include <set>
# include <vector>
# include <algorithm>
# include <iterator>
# include <list>

/* System */
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <unistd.h>
// # include <machine/types.h>
# include <signal.h>

/* Network */
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/select.h>
# include <arpa/inet.h>

# include "ConfigParser.hpp"
# include "ConfigFile.hpp"
# include "ServerConfig.hpp"
# include "Location.hpp"
# include "HttpRequest.hpp"
# include "CgiHandler.hpp"
# include "Mime.hpp"
# include "Logger.hpp"

/**
 * @file Webserv.hpp
 * @brief Central include file for the webserver project, incorporating necessary headers, utilities, and constants.
 *
 * This file acts as a central hub for including all necessary headers and defining constants and utility functions used
 * throughout the webserver project. It includes standard C++ libraries, system headers, network libraries, and custom
 * project-specific headers. It also defines key constants related to server operation, such as timeouts and buffer sizes.
 * Additionally, utility functions for common tasks such as string conversion and status code handling are declared here.
 */

/* Constants */
#define CONNECTION_TIMEOUT 60 // Time in seconds before a client is disconnected due to inactivity.
#ifdef TESTER
    #define MESSAGE_BUFFER 40000 // Buffer size for message handling (testing mode).
#else
    #define MESSAGE_BUFFER 40000 // Buffer size for message handling (normal mode).
#endif

#define MAX_URI_LENGTH 4096           // Maximum allowed length for a URI.
#define MAX_CONTENT_LENGTH 30000000   // Maximum allowed content length for a request body.

/**
 * @brief Converts a value of any type to a string.
 *
 * @tparam T The type of the value to be converted.
 * @param val The value to be converted to a string.
 * @return A string representation of the value.
 */
template <typename T>
std::string toString(const T val)
{
    std::stringstream stream;
    stream << val;
    return stream.str();
}

/* Utility functions declarations */

/**
 * @brief Get the string representation of an HTTP status code.
 *
 * @param status_code The HTTP status code.
 * @return A string describing the status code.
 */
std::string statusCodeString(short status_code);

/**
 * @brief Retrieve the error page content for a given HTTP status code.
 *
 * @param status_code The HTTP status code.
 * @return A string containing the HTML content of the error page.
 */
std::string getErrorPage(short status_code);

/**
 * @brief Build an HTML index page based on a directory listing.
 *
 * @param directory The directory path.
 * @param content A vector to store the HTML content.
 * @param content_length The total length of the generated content.
 * @return An integer indicating success (0) or failure (-1).
 */
int buildHtmlIndex(std::string &directory, std::vector<uint8_t> &content, size_t &content_length);

/**
 * @brief Convert a string to an integer.
 *
 * @param str The string to convert.
 * @return The integer value of the string.
 */
int ft_stoi(std::string str);

/**
 * @brief Convert a hexadecimal string to a decimal integer.
 *
 * @param nb The hexadecimal string.
 * @return The decimal integer value.
 */
unsigned int fromHexToDec(const std::string& nb);

#endif // WEBSERV_HPP

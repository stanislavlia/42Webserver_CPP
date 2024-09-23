/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/23 12:59:56 by moetienn          #+#    #+#             */
/*   Updated: 2024/09/23 13:46:36 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>


// CANONICAL FORM

RequestHandler::RequestHandler(int socket, Request& request, ServerParam& config) : _request(request), _config(config), _socket(socket)
{
}

RequestHandler::RequestHandler(const RequestHandler& src) : _request(src._request), _config(src._config), _socket(src._socket)
{
}

RequestHandler& RequestHandler::operator=(const RequestHandler& rhs)
{
	if (this != &rhs) {
		_request = rhs._request;
		_config = rhs._config;
		_socket = rhs._socket;
	}
	return *this;
}

RequestHandler::~RequestHandler()
{
}

// END CANONICAL FORM

std::string	RequestHandler::_render_html(const std::string& path)
{
    std::ifstream file(path.c_str());

    if (!file.is_open())
    {
        Logger::logMsg(ERROR, "Failed to open HTML file: %s", path.c_str());
        return _render_html("www/error/not_found.html");
    }

    std::stringstream stream_buffer;
    stream_buffer << file.rdbuf();
    return stream_buffer.str();
}

void RequestHandler::_respond_with_error(int socket, int status_code, const std::string& status_message)
{
	std::string path = _config.getErrorPage().at(status_code);

	std::string html_content = _render_html(path);
	std::stringstream ss;
    ss << html_content.length();

    std::stringstream status_code_ss;
    status_code_ss << status_code;

    std::string response = "HTTP/1.1 " + status_code_ss.str() + " " + status_message + "\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: " + ss.str() + "\r\n"
                           "\r\n" + html_content;

    send(socket, response.c_str(), response.length(), 0);
}


void	RequestHandler::_respond_with_html(int socket, const std::string& path, int status_code, const std::string& status_message)
{
    std::string html_content = _render_html(path); //need to add exceptions

    std::stringstream ss;
    ss << html_content.length();

    std::stringstream status_code_ss;
    status_code_ss << status_code;

    std::string response = "HTTP/1.1 " + status_code_ss.str() + " " + status_message + "\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: " + ss.str() + "\r\n"
                           "\r\n" + html_content;

    send(socket, response.c_str(), response.length(), 0);
}

void	RequestHandler::_handleInvalidRequest(int client_fd, int validation_code)
{
    if (validation_code == 1 || validation_code == 3)
    {
        // 400 Bad Request
        _respond_with_error(client_fd, 400, "Bad Request");
        Logger::logMsg(ERROR, "Bad Request %d - code", 400);
    }
    else if (validation_code == 2)
    {
        // 405 Method Not Allowed
        _respond_with_error(client_fd, 405, "Method Not Allowed");
        Logger::logMsg(ERROR, "Method Not Allowed %d - code", 405);
    }
}

std::string	_generateDirectoryListing(const std::string& path, const std::string& uri)
{
	std::cout << "IN GENERATE DIRECTORY LISTING" << std::endl;
	std::stringstream html;
    html << "<html><head><title>Directory Listing</title></head><body>";
    html << "<h1>Directory Listing for " << path << "</h1>";
    html << "<ul>";

    DIR* dir = opendir(path.c_str());
    if (dir)
    {
        struct dirent* entry;
		
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_name[0] != '.') // Skip hidden files
            {
				if (entry->d_type == DT_DIR)
				{
					std::string file_path = uri + entry->d_name;
					html << "<li><a href=\"" << file_path << "\">" << entry->d_name << "/</a></li>";
				}
				else
				{
					std::string file_path = uri + "/" + entry->d_name;
                	html << "<li><a href=\"" << file_path << "\">" << entry->d_name << "</a></li>";
				}
            }
        }
        closedir(dir);
    }

    html << "</ul></body></html>";
    return (html.str());
}

void	RequestHandler::_serveHtmlContent(int client_fd, const std::string& html_content, int status_code, const std::string& status_message)
{
    std::cout << "IN SERVE HTML CONTENT" << std::endl;

    std::stringstream ss;
    ss << html_content.length();

    std::stringstream status_code_ss;
    status_code_ss << status_code;

    std::string response = "HTTP/1.1 " + status_code_ss.str() + " " + status_message + "\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: " + ss.str() + "\r\n"
                           "\r\n" + html_content;

    send(client_fd, response.c_str(), response.length(), 0);
}

void	RequestHandler::_handleDirectoryListing(int client_fd, const std::string& path, const std::string& uri)
{
    std::string html_content = _generateDirectoryListing(path, uri);
	_serveHtmlContent(client_fd, html_content, 200, "OK");
}

void	DefaultErrorPage(int client_fd, int status_code)
{
	std::stringstream status_code_ss;
    status_code_ss << status_code;

    std::string response = "HTTP/1.1 " + status_code_ss.str() + " ""\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: 0\r\n"
                           "\r\n";

    send(client_fd, response.c_str(), response.length(), 0);
}

void	RequestHandler::_handleRootDirectoryRequest(int client_fd, const std::string& rootDir, const std::string& uri)
{
    if (_config.getIndex().empty() && _config.getAutoIndex() == 1)
    {
        _handleDirectoryListing(client_fd, rootDir, uri);
    }
    else if (_config.getIndex().empty() && _config.getAutoIndex() == 0)
    {
        try
        {
        	_respond_with_error(client_fd, 403, "Forbidden");
        }
        catch (std::exception& e)
        {
            DefaultErrorPage(client_fd, 403);
        }
    }
    else
    {
        _respond_with_html(client_fd, rootDir + _config.getIndex(), 200, "OK");
    }
}

void	RequestHandler::_handleSpecificUriRequest(int client_fd, const std::string& rootDir, const std::string& uri)
{
    if (uri == "/home")
    {
        _respond_with_html(client_fd, rootDir + "/static/home.html", 200, "OK");
    }
}

void	RequestHandler::_handleFileOrDirectoryRequest(int client_fd, const std::string& full_path, const std::string& uri)
{
    struct stat path_stat;
    if (stat(full_path.c_str(), &path_stat) == 0)
    {
        std::cout << "PATH STAT: " << path_stat.st_mode << std::endl;
        if (S_ISDIR(path_stat.st_mode))
        {
            if (_config.getAutoIndex() == 1)
            {
                _handleDirectoryListing(client_fd, full_path, uri);
            }
            else
            {
               _respond_with_error(client_fd, 403, "Forbidden");
            }
        }
        else if (S_ISREG(path_stat.st_mode))
        {
            _respond_with_html(client_fd, full_path.c_str(), 200, "OK");
        }
        else
        {
            _respond_with_error(client_fd, 404, "Not Found");
            Logger::logMsg(ERROR, "No page FOUND %d - code", 404);
        }
    }
    else
    {
        _respond_with_html(client_fd, _config.getRoot() + "/error/not_found.html", 404, "Not Found");
        Logger::logMsg(ERROR, "No page FOUND %d - code", 404);
    }
}

void	RequestHandler::handleRequest()
{
	if (_request.isValid() != 0)
	{
		_handleInvalidRequest(_socket, _request.isValid());
	}
	else
	{
		std::string full_path = _config.getRoot() + _request.getUri(); // for autoindex on 
		std::string rootDir = _config.getRoot();

		std::cout << "FULL PATH: " << full_path << std::endl;

		if (_request.getMethod() == "GET" && _request.getUri() == "/")
		{
		    _handleRootDirectoryRequest(_socket, rootDir, _request.getUri());
		}
		else
		{
		    _handleSpecificUriRequest(_socket, rootDir, _request.getUri());
		    _handleFileOrDirectoryRequest(_socket, full_path, _request.getUri());
		}
	}
}
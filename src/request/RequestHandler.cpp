/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/23 12:59:56 by moetienn          #+#    #+#             */
/*   Updated: 2024/10/18 09:52:28 by moetienn         ###   ########.fr       */
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

void RequestHandler::_respond_with_error(int socket, int status_code, const std::string& status_message, const Location& location)
{
	std::string path = location.getErrorPage().at(status_code);


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

void	RequestHandler::_handleInvalidRequest(int client_fd, int validation_code, const Location& location)
{
	if (validation_code == 1 || validation_code == 3)
	{
		// 400 Bad Request
		try {
			_respond_with_error(client_fd, 400, "Bad Request", location);
		}
		catch (std::exception& e)
		{
			DefaultErrorPage(client_fd, 400);
		}
		Logger::logMsg(ERROR, "Bad Request %d - code", 400);
	}
	else if (validation_code == 2)
	{
		// 405 Method Not Allowed
		try
		{
			_respond_with_error(client_fd, 405, "Method Not Allowed", location);
		}
		catch (std::exception& e)
		{
			DefaultErrorPage(client_fd, 405);
		}
		Logger::logMsg(ERROR, "Method Not Allowed %d - code", 405);
	}
}

std::string	_generateDirectoryListing(const std::string& path, const std::string& uri)
{
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


void	RequestHandler::_handleRootDirectoryRequest(int client_fd, const std::string& rootDir, const std::string& uri, const Location& location)
{
	if (location.getIndex().empty() && location.getAutoIndex() == 1)
	{
		_handleDirectoryListing(client_fd, rootDir, uri);
	}
	else if (location.getIndex().empty() && location.getAutoIndex() == 0)
	{
		try
		{
			_respond_with_error(client_fd, 403, "Forbidden", location);
		}
		catch (std::exception& e)
		{
			DefaultErrorPage(client_fd, 403);
		}
	}
	else
	{
		_respond_with_html(client_fd, rootDir + location.getIndex(), 200, "OK");
	}
}

void	RequestHandler::_handleSpecificUriRequest(int client_fd, const std::string& rootDir, const std::string& uri)
{
	if (uri == "/home")
	{
		_respond_with_html(client_fd, rootDir + "/static/home.html", 200, "OK");
	}
}

void	RequestHandler::_handleFileOrDirectoryRequest(int client_fd, const std::string& full_path, const std::string& uri, const Location& location)
{
	struct stat path_stat;
	if (stat(full_path.c_str(), &path_stat) == 0)
	{
		if (S_ISDIR(path_stat.st_mode))
		{
			if (location.getAutoIndex() == 1)
			{
				_handleDirectoryListing(client_fd, full_path, uri);
			}
			else
			{
				try
				{
					_respond_with_error(client_fd, 403, "Forbidden", location);
				}
				catch (std::exception& e)
				{
					DefaultErrorPage(client_fd, 403);
				}
			}
		}
		else if (S_ISREG(path_stat.st_mode))
		{
			std::cout << "IS REG" << std::endl;
			if (access(full_path.c_str(), R_OK) == 0)
				_respond_with_html(client_fd, full_path.c_str(), 200, "OK");
			else
			{
				try
				{
					_respond_with_error(client_fd, 403, "Forbidden", location);
				}
				catch (std::exception& e)
				{
					DefaultErrorPage(client_fd, 403);
				}
			}
		}
		else
		{
			try
			{
				_respond_with_error(client_fd, 404, "Not Found", location);
			}
			catch (std::exception& e)
			{
				DefaultErrorPage(client_fd, 404);
			}
			Logger::logMsg(ERROR, "No page FOUND %d - code", 404);
		}
	}
	else
	{
		try {
			_respond_with_error(client_fd, 404, "Not Found", location);
		}
		catch (std::exception& e)
		{
			DefaultErrorPage(client_fd, 404);
		}
		Logger::logMsg(ERROR, "No page FOUND %d - code", 404);
	}
}

void	restore_locations_order(std::vector<Location>& locations, std::vector<std::pair<size_t, Location*> >& indexed_locations)
{
	for (size_t i = 0; i < locations.size(); ++i)
	{
		locations[i] = *indexed_locations[i].second;
		std::cout << "location[i] root: " << locations[i].getRoot() << " Indexed locations second: " << indexed_locations[i].second->getRoot() << std::endl;
	}
	for (size_t i = 0; i < locations.size(); ++i)
	{
		std::cout << "Restored location: " << locations[i].getRoot() << std::endl;
	}
}

bool	compareLocations(const std::pair<size_t, Location>& a, const std::pair<size_t, Location>& b)
{
    return a.second.getRoot().length() > b.second.getRoot().length();
}

// Comparator function to sort locations by their original indices
bool	compareOriginalIndices(const std::pair<size_t, Location>& a, const std::pair<size_t, Location>& b)
{
    return a.first < b.first;
}

// Function to check if a file or directory exists
bool	fileOrDirectoryExists(const std::string& path, bool& is_directory)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
	{
        return false; // Path does not exist
    }
    is_directory = (info.st_mode & S_IFDIR) != 0;
    return true;
}

void	restoreOriginalOrder(std::vector<std::pair<size_t, Location> >& indexed_locations, std::vector<Location>& locations)
{
    // Restore original order of locations
    std::sort(indexed_locations.begin(), indexed_locations.end(), compareOriginalIndices);

    // Update locations with the restored order
    for (size_t i = 0; i < indexed_locations.size(); ++i)
	{
        locations[i] = indexed_locations[i].second;
        std::cout << "Restored location: " << locations[i].getRoot() << std::endl;
    }
}

void	RequestHandler::handleRequest()
{
    std::string request_uri;
    if (_request.getUri() == "/")
	{
        std::cout << "ROOT URI" << std::endl;
        request_uri = _config.locations[0].getRoot() + _config.locations[0].getIndex();
    }
	else
	{
        std::cout << "NOT ROOT URI" << std::endl;
        request_uri = "www" + _request.getUri(); // Add the www prefix
    }

    std::string request_method = _request.getMethod();
    std::string matched_root;
    Location matched_location;
    bool location_found = false;
    size_t found = 0;

    std::cout << "REQUEST URI: " << request_uri << std::endl;

    // Store original indices and copies of locations
    std::vector<std::pair<size_t, Location> > indexed_locations;
    for (size_t i = 0; i < _config.locations.size(); ++i) {
        indexed_locations.push_back(std::make_pair(i, _config.locations[i]));
        std::cout << "Index location: " << indexed_locations[i].second.getRoot() << std::endl;
    }

    // Sort locations by root length in descending order
    std::sort(indexed_locations.begin(), indexed_locations.end(), compareLocations);

	// Check if the URI matches any of the locations
    for (size_t i = 0; i < indexed_locations.size(); ++i)
    {
        const std::string& location_root = indexed_locations[i].second.getRoot();
        std::cout << "location root: " << location_root << std::endl;
        
        if (request_uri.find(location_root) == 0) // Check if the URI starts with the location root
        {
            std::string full_path = location_root + request_uri.substr(location_root.length());
            std::cout << "full path: " << full_path << std::endl;
            bool is_directory = false;
            if (fileOrDirectoryExists(full_path, is_directory))
			{
                matched_root = location_root;
                matched_location = indexed_locations[i].second;
                location_found = true;
                found = indexed_locations[i].first; // Store the original index
                break;
            }
        }
    }

    // Restore original order of locations
	restoreOriginalOrder(indexed_locations, _config.locations);

    if (!location_found)
    {
		// find the closest correct location ex : www/static/allo.html = nearest location is www/static
		
		
		
		
        // std::cerr << "Error: No matching location found for URI: " << request_uri << std::endl;
        _respond_with_error(_socket, 404, "Not Found", matched_location);
        return;
    }

    // Check if the request method is allowed
    std::vector<std::string> allowed_methods = _config.locations[found].getAllowedMethods();
    bool method_allowed = false;

    for (size_t i = 0; i < allowed_methods.size(); ++i)
	{
        if (request_method == allowed_methods[i])
		{
            method_allowed = true;
            break;
        }
    }

    if (!method_allowed)
	{
        try {
            _respond_with_error(_socket, 405, "Method Not Allowed", matched_location);
        }
        catch (std::exception& e)
        {
            DefaultErrorPage(_socket, 405);
        }
        return;
    }

    std::string full_path = matched_root + request_uri.substr(matched_root.length());

    bool is_directory = false;
    if (!fileOrDirectoryExists(full_path, is_directory))
	{
        try
		{
            _respond_with_error(_socket, 404, "Not Found", matched_location);
        }
        catch (std::exception& e)
        {
            DefaultErrorPage(_socket, 404);
        }
        return;
    }

    if (request_method == "POST")
    {
        std::cout << "Handle Request >POST< : " << request_uri << std::endl;
        std::cout << "BODY : " << _request.getBody() << std::endl;
        // Handle POST request
    }
    else if (request_method == "GET" && request_uri == "/index.html")
    {
        std::cout << "Handle Request >Root< : " << request_uri << std::endl;
        _handleRootDirectoryRequest(_socket, matched_root, request_uri, matched_location);
    }
    else
    {
        std::cout << "Handle Request >Request< : " << request_uri << std::endl;
        _handleSpecificUriRequest(_socket, matched_root, request_uri);
        _handleFileOrDirectoryRequest(_socket, full_path, request_uri, matched_location);
    }
}
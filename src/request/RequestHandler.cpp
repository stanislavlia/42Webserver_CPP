/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/23 12:59:56 by moetienn          #+#    #+#             */
/*   Updated: 2025/01/06 23:07:53 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"

// CANONICAL FORM

RequestHandler::RequestHandler(int socket, Request& request, ServerParam& config, Monitor *mon) : _request(request), _config(config), _socket(socket), monitor(mon)
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

void	RequestHandler::_DefaultErrorPage(int status_code)
{
	std::stringstream status_code_ss;
	status_code_ss << status_code;

	response = "HTTP/1.1 " + status_code_ss.str() + " ""\r\n"
						   "Content-Type: text/html\r\n"
						   "Content-Length: 0\r\n"
						   "\r\n";

}

std::string RequestHandler::getResponse() const
{
    return response;
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

void RequestHandler::_respond_with_error(int status_code, const std::string& status_message, const Location& location)
{
	std::string path = location.getErrorPage().at(status_code);

	std::string html_content = _render_html(path);
	std::stringstream ss;
	ss << html_content.length();

	std::stringstream status_code_ss;
	status_code_ss << status_code;

	response = "HTTP/1.1 " + status_code_ss.str() + " " + status_message + "\r\n"
						   "Content-Type: text/html\r\n"
						   "Content-Length: " + ss.str() + "\r\n"
						   "\r\n" + html_content;

}


void	RequestHandler::_respond_with_html(const std::string& path, int status_code, const std::string& status_message)
{
	std::string html_content = _render_html(path); //need to add exceptions

	std::stringstream ss;
	ss << html_content.length();

	std::stringstream status_code_ss;
	status_code_ss << status_code;

	response = "HTTP/1.1 " + status_code_ss.str() + " " + status_message + "\r\n"
						   "Content-Type: text/html\r\n"
						   "Content-Length: " + ss.str() + "\r\n"
						   "\r\n" + html_content;

}

void	RequestHandler::_handleInvalidRequest(int validation_code, const Location& location)
{
	std::cout << "Validation code: " << validation_code << std::endl;
	if (validation_code == 400)
	{
		validation_code = 1;
	}
	else if (validation_code == 405)
	{
		validation_code = 2;
	}
	if (validation_code == 1 || validation_code == 3)
	{
		// 400 Bad Request
		try {
			_respond_with_error(400, "Bad Request", location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(400);
		}
		Logger::logMsg(ERROR, "Bad Request %d - code", 400);
	}
	else if (validation_code == 2)
	{
		// 405 Method Not Allowed
		try
		{
			_respond_with_error(405, "Method Not Allowed", location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(405);
		}
		Logger::logMsg(ERROR, "Method Not Allowed %d - code", 405);
	}
}

void	RequestHandler::_serveHtmlContent(const std::string& html_content, int status_code, const std::string& status_message)
{
	std::stringstream ss;
	ss << html_content.length();

	std::stringstream status_code_ss;
	status_code_ss << status_code;

	response = "HTTP/1.1 " + status_code_ss.str() + " " + status_message + "\r\n"
						   "Content-Type: text/html\r\n"
						   "Content-Length: " + ss.str() + "\r\n"
						   "\r\n" + html_content;

}

bool	compareLocations(const std::pair<size_t, Location>& a, const std::pair<size_t, Location>& b)
{
	return a.second.getLocationName().length() > b.second.getLocationName().length();
}

bool	RequestHandler::findMatchingLocation(const std::string& request_uri, Location& matched_location, size_t& matched_index)
{
	bool location_found;

	location_found = false;

	if (_config.locations.size() == 0)
	{
		return false;
	}

	std::vector<std::pair<size_t, Location> > indexed_locations;
    for (size_t i = 0; i < _config.locations.size(); ++i) 
    {
        indexed_locations.push_back(std::make_pair(i, _config.locations[i]));
    }

	std::sort(indexed_locations.begin(), indexed_locations.end(), compareLocations);

	// search by location name (static)
	for (size_t i = 0; i < indexed_locations.size(); ++i)
	{
		const std::string& location_name = indexed_locations[i].second.getLocationName();
		if (request_uri.find(location_name) == 0)
		{
			matched_location = indexed_locations[i].second;
			matched_index = indexed_locations[i].first;
			location_found = true;
			return location_found;
		}
	}
	return location_found;
}


std::string RequestHandler::buildRequestPath(const Location& location, std::string& request_uri)
{
    std::string root = location.getRoot();
    std::string location_name = location.getLocationName();
    std::string adjusted_uri = request_uri;

    // Check if the request URI starts with the location name
    if (request_uri.find(location_name) == 0)
	{
        // Remove the location name from the request URI
		if (location_name != "/")
        	adjusted_uri = request_uri.substr(location_name.length());
    }

    // If the adjusted URI is empty or "/", return the root or the root plus index
    if (adjusted_uri.empty() || adjusted_uri == "/")
	{
        if (location.getIndex().empty())
		{
            return root;
        }
		else
		{
            return root + location.getIndex();
        }
    }

	struct stat path_stat;

	// Check if the adjusted URI is a directory
	if (stat((root + adjusted_uri).c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode))\
	{
		// Check if the directory has an index file
		if (!location.getIndex().empty())
		{
			// std::cout << "INDEX IS NOT EMPTY" << std::endl;
			std::string index_path = root + adjusted_uri + location.getIndex();
			return index_path;
		}
	}
	

    // // Construct the full path
	std::string full_path;
    full_path = root + adjusted_uri;


    return full_path;
}

void	RequestHandler::handleRequest(std::vector<int>& client_fds, int client_fd) 
{
    std::string request_uri = _request.getUri();
    std::string request_method = _request.getMethod();
    Location matched_location;
    size_t matched_index;


    // Find the matching location
    bool location_found = findMatchingLocation(request_uri, matched_location, matched_index);

    if (!location_found) 
    {
        std::cerr << "Error: No matching location found for URI: " << request_uri << std::endl;
		try
		{
			_respond_with_error(404, "Not Found", matched_location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(404);
		}
        return;
    }


    // Check if the request method is allowed
    std::vector<std::string> allowed_methods = _config.locations[matched_index].getAllowedMethods();
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
        try 
        {
            _respond_with_error(405, "Method Not Allowed", matched_location);
        } 
        catch (std::exception& e) 
        {
            _DefaultErrorPage(405);
        }
		Logger::logMsg(ERROR, "Method Not Allowed %d - code", 405);
        return;
    }

    std::string full_path = buildRequestPath(matched_location, request_uri);

    if (matched_location.getRoot() == "www/cgi-bin") 
    {
        _handleCgiRequest(full_path, matched_location, request_uri, client_fd, client_fds);
    }
    else if (request_method == "POST")
    {
        _handlePostRequest(full_path, matched_location);
    } 
    else if (request_method == "DELETE") 
    {
        _handleDeleteRequest(full_path, matched_location);
    } 
    else if (request_method == "GET")
    {
        _handleFileOrDirectoryRequest(full_path, request_uri, matched_location);
    }
}
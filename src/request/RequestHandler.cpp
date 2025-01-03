/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/23 12:59:56 by moetienn          #+#    #+#             */
/*   Updated: 2024/12/24 14:20:43 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"

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

std::string	RequestHandler::getResponse() const
{
	return response;
}

void	RequestHandler::_DefaultErrorPage(int status_code)
{
	std::stringstream status_code_ss;
	status_code_ss << status_code;

	response = "HTTP/1.1 " + status_code_ss.str() + " ""\r\n"
						   "Content-Type: text/html\r\n"
						   "Content-Length: 0\r\n"
						   "\r\n";

	// send(client_fd, response.c_str(), response.length(), 0);
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

	// std::cout << "Error page path: " << path << std::endl;
	std::string html_content = _render_html(path);
	std::stringstream ss;
	ss << html_content.length();

	std::stringstream status_code_ss;
	status_code_ss << status_code;

	response = "HTTP/1.1 " + status_code_ss.str() + " " + status_message + "\r\n"
						   "Content-Type: text/html\r\n"
						   "Content-Length: " + ss.str() + "\r\n"
						   "\r\n" + html_content;

	// send(socket, response.c_str(), response.length(), 0);
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

	// send(socket, response.c_str(), response.length(), 0);
}

void	RequestHandler::_handleInvalidRequest(int validation_code, const Location& location)
{
	// std::cout << "Validation code: " << validation_code << std::endl;
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

	// send(client_fd, response.c_str(), response.length(), 0);
}

void	restore_locations_order(std::vector<Location>& locations, std::vector<std::pair<size_t, Location*> >& indexed_locations)
{
	for (size_t i = 0; i < locations.size(); ++i)
	{
		locations[i] = *indexed_locations[i].second;
		// std::cout << "location[i] root: " << locations[i].getRoot() << " Indexed locations second: " << indexed_locations[i].second->getRoot() << std::endl;
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

void	restoreOriginalOrder(std::vector<std::pair<size_t, Location> >& indexed_locations, std::vector<Location>& locations)
{
	// Restore original order of locations
	std::sort(indexed_locations.begin(), indexed_locations.end(), compareOriginalIndices);

	// Update locations with the restored order
	for (size_t i = 0; i < indexed_locations.size(); ++i)
	{
		locations[i] = indexed_locations[i].second;
	}
}

void	RequestHandler::handleRequest()
{
	std::string request_uri;

	if (_request.getUri() == "/")
		request_uri = _config.locations[0].getRoot() + _config.locations[0].getIndex();
	else
		request_uri = "www" +_request.getUri(); // Add the www prefix

	std::string request_method = _request.getMethod();
	std::string matched_root;
	Location matched_location;
	bool location_found = false;
	size_t found = 0;

	// Store original indices and copies of locations
	std::vector<std::pair<size_t, Location> > indexed_locations;
	for (size_t i = 0; i < _config.locations.size(); ++i)
	{
		indexed_locations.push_back(std::make_pair(i, _config.locations[i]));
	}

	// Sort locations by root length in descending order
	std::sort(indexed_locations.begin(), indexed_locations.end(), compareLocations);

	// Check if the URI matches any of the locations
	for (size_t i = 0; i < indexed_locations.size(); ++i)
	{
		const std::string& location_root = indexed_locations[i].second.getRoot();
		
		if (request_uri.find(location_root) == 0) // Check if the URI starts with the location root
		{
			std::string full_path = location_root + request_uri.substr(location_root.length());
			matched_root = location_root;
			matched_location = indexed_locations[i].second;
			found = indexed_locations[i].first; // Store the original index
			if (request_uri == location_root)
			{
				location_found = true;
				break;
			}
			break;
		}
	}

	// Restore original order of locations
	restoreOriginalOrder(indexed_locations, _config.locations);

	if (!location_found)
	{
		std::string trimmed_uri = request_uri;
		size_t pos = trimmed_uri.find_last_of('/');
		trimmed_uri = trimmed_uri.substr(0, pos);
		while (!trimmed_uri.empty() && !location_found)
		{
			for (size_t i = indexed_locations.size() - 1; i > 0; --i)
			{
				const std::string& location_root = indexed_locations[i].second.getRoot();
				if (trimmed_uri == location_root)
				{
					// exit(0);
					matched_root = location_root;
					matched_location = indexed_locations[i].second;
					location_found = true;
					found = indexed_locations[i].first; // Store the original index
					break;
				}
			}
			pos = trimmed_uri.find_last_of('/');
			if (pos == std::string::npos)
			{
				break;
			}
			trimmed_uri = trimmed_uri.substr(0, pos);
		}
		if (!location_found)
		{
			std::cerr << "Error: No matching location found for URI: " << request_uri << std::endl;
			_respond_with_error(404, "Not Found", matched_location);
			return;
		}
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
			_respond_with_error(405, "Method Not Allowed", matched_location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(405);
		}
		return;
	}

	std::string full_path = matched_root + request_uri.substr(matched_root.length());

	if (matched_root == "www/cgi-bin")
	{
		_handleCgiRequest(full_path, matched_location, request_uri);
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

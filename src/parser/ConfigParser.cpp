/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 11:52:22 by moetienn          #+#    #+#             */
/*   Updated: 2025/01/28 05:03:24 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

// CANONICAL FORM

ConfigParser::ConfigParser(const std::string& configFile) : _configFile(configFile)
{
}

ConfigParser::ConfigParser(ConfigParser const &src)
{
	*this = src;
}

ConfigParser &ConfigParser::operator=(ConfigParser const &rhs)
{
	if (this != &rhs) {
		this->_configFile = rhs._configFile;
		this->_servers = rhs._servers;
	}
	return *this;
}

ConfigParser::~ConfigParser()
{
}

// HELPERS FUNCTIONS

void    parseListen(std::istringstream& iss, ServerParam& server)
{
	int listen;
	iss >> listen;
	server.setListen(listen);
}

void    parseServerName(std::istringstream& iss, ServerParam& server)
{
	std::string server_name;
	std::getline(iss, server_name, ';');
	server_name = server_name.substr(2);
	server.setServerName(server_name);
}

void parseIndex(std::istringstream& iss, Location& location)
{
	std::string index;
	std::getline(iss, index, ';');
	index.erase(0, index.find_first_not_of(' ')); // Remove leading spaces
	location.setIndex(index);
}

void	parseAutoIndex(std::istringstream& iss, Location& location)
{
	std::string line;
	std::getline(iss, line, ';');
	line = line.substr(1);

	if (line == "on")
		location.setAutoIndex(true);
	else if (line == "off")
		location.setAutoIndex(false);
	else
		location.setAutoIndex(false);
}


void	parseLocationName(std::istringstream& iss, Location& location)
{
	std::string location_full;
	std::string location_name;
	std::getline(iss, location_name, ';');
	location_full = location_name.substr(1);
	location_name = location_full.substr(0, location_full.size() - 1);
	location_name.erase(location_name.find_last_not_of(" \t\n\r\f\v") + 1);
	location.setLocationName(location_name);
}

void    parseRoot(std::istringstream& iss, ServerParam& server)
{
	std::string root;
	std::getline(iss, root, ';');
	root = root.substr(1);
	server.setRoot(root);
}

void	parseRoot(std::istringstream& iss, Location& location)
{
	std::string root;
	std::getline(iss, root, ';');
	root = root.substr(1);
	location.setRoot(root);
}

void	parseAllowedMethods(std::istringstream& iss, Location& location)
{
	std::string methods;
	std::getline(iss, methods, ';');
	methods = methods.substr(1);
	std::vector<std::string> allowed_methods;
	std::string method;
	std::istringstream iss_methods(methods);
	while (std::getline(iss_methods, method, ' '))
	{
		allowed_methods.push_back(method);
	}
	location.setAllowedMethods(allowed_methods);
}

void	parseErrorPage(std::istringstream& iss, Location& location)
{
	std::string error_code_str;
	std::string error_path;

	iss >> error_code_str;
	std::getline(iss, error_path, ';');

	error_path = error_path.substr(1);

	int error_code = std::atoi(error_code_str.c_str());

	location.setErrorPage(error_code, error_path);
}

void	parseClientMaxBodySize(std::istringstream& iss, ServerParam& server)
{
	int client_max_body_size;
	iss >> client_max_body_size;
	server.setClientMaxBodySize(client_max_body_size);
}

void	parseHost(std::istringstream& iss, ServerParam& server)
{
	std::string host;
	std::getline(iss, host, ';');
	host = host.substr(1);
	server.setHost(host.c_str());
}
void	parseCgiPath(std::istringstream& iss, Location& location)
{
	std::string cgi_path;
	std::getline(iss, cgi_path, ';');
	cgi_path = cgi_path.substr(1);
	location.setCgiPath(cgi_path);
}

void	parseReturn(std::istringstream& iss, Location& location)
{
	std::string return_path;
	std::getline(iss, return_path, ';');
	return_path = return_path.substr(1);
	location.setReturn(return_path);
}

void parseCgiExtension(std::istringstream& iss, Location& location)
{
    std::string extensions;
    std::getline(iss, extensions, ';');
    extensions = extensions.substr(1);
    std::vector<std::string> allowed_extensions;
    std::string extension;
    std::istringstream iss_extensions(extensions);
    while (std::getline(iss_extensions, extension, ' '))
    {
        if (!extension.empty())
            allowed_extensions.push_back(extension);
    }
    location.setAllowedExtension(allowed_extensions);
	
}

void parseCgiInterperter(std::istringstream& iss, Location& location)
{
    std::string line;
    std::getline(iss, line, ';');
    line = line.substr(1);
    
    std::istringstream iss_line(line);
    std::string extension, interpreter;
    
    iss_line >> extension >> interpreter;
    
    if (!extension.empty() && !interpreter.empty()) {
        location.AddCgiInterpreter(extension, interpreter);
    }
}

// MAIN FUNCTION

/**
 * Parse the config file and return a vector of ServerParam
 * Use the following directives:
 * - listen
 * - server_name
 * - index
 * - error_page
 * - location
 * - server
 * - autoindex
 * - root
 * - allowed_methods
 * - cgi_path
 * - }
 * 
 *  @return std::vector<ServerParam> servers
 */

std::vector<ServerParam>    ConfigParser::parse()
{
	// std::vector<ServerParam> servers;
	
	std::ifstream file(_configFile.c_str());

	if (!file.is_open())
	{
		throw std::runtime_error("Unable to open config file: " + _configFile);
	}
	
	std::string tokens[TOKEN_COUNT] = {"listen", "server_name", "Host" ,"client_max_body_size" , "index", "error_page", "location", "cgi_path", "autoindex", "root", "allowed_methods" , "server", "return", "cgi_extension", "cgi_interpreter" , "}"};
	void (*functions[TOKEN_COUNT])(std::istringstream&, ServerParam&) = {parseListen, parseServerName, parseHost, parseClientMaxBodySize};
	void (*functions_location[TOKEN_COUNT])(std::istringstream&, Location&) = {parseIndex, parseAutoIndex, parseRoot, parseAllowedMethods, parseErrorPage, parseLocationName, parseCgiPath, parseReturn, parseCgiExtension, parseCgiInterperter};
	

	std::string line;
	ServerParam current_server;
	Location	current_location;
	bool in_server_block = false;
	bool in_location_block = false;


	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string token;
		iss >> token;
		if (token.empty())
			continue ;
	
		bool token_found = false;
		for (int i = 0; i < TOKEN_COUNT; ++i)
		{
			if (token == tokens[i])
			{
				token_found = true;
				switch (static_cast<FunctionType>(i))
				{
					case LISTEN:
						functions[0](iss, current_server);
						break;
					case SERVER_NAME:
						functions[1](iss, current_server);
						break;
					case HOST_NAME:
						functions[2](iss, current_server);
						break;
					case CLIENT_MAX_BODY_SIZE:
						functions[3](iss, current_server);
						break;
					case INDEX:
						if (in_location_block)
							functions_location[0](iss, current_location);
						break;
					case ERROR_PAGE:
						functions_location[4](iss, current_location);
						break;
					case LOCATION:
					{
						in_location_block = true;
						current_location = Location();
						functions_location[5](iss, current_location);
					}
						break;
					case CGI_PATH:
					{
						if (in_location_block)
							functions_location[6](iss, current_location);
					}
						break;
					case CGI_EXTENSION:
					{
						if (in_location_block)
							functions_location[8](iss, current_location);
					}
						break;
					case CGI_INTERPRETER:
					{
						if (in_location_block)
							functions_location[9](iss, current_location);
					}
						break;
					case AUTOINDEX:
					{
						if (in_location_block)
							functions_location[1](iss, current_location);
					}
						break;
					case ROOT:
					{
						if (in_location_block)
							functions_location[2](iss, current_location);
					}
						break;
					case ALLOWED_METHODS:
					{
						if (in_location_block)
							functions_location[3](iss, current_location);
					}
						break;
					case RETURN:
						if (in_location_block)
							functions_location[7](iss, current_location);
						break;
					case SERVER:
						if (in_server_block)
						{
							_servers.push_back(current_server);
							current_server = ServerParam();
						}
						in_server_block = true;
						break;
					case BRACKET:
						if (in_location_block)
						{
							current_server.addLocation(current_location);
							in_location_block = false;
						}
						else if (in_server_block)
						{
							_servers.push_back(current_server);
							current_server = ServerParam();
							in_server_block = false;
						}
						break;
					case UNKNOWN:
						throw std::runtime_error("Unknown directive: " + token);
						break;
					case TOKEN_COUNT:
						break;
				}
				break;
			}
		}
		if (!token_found)
		{
			throw std::runtime_error("Unknown directive: " + token);    
		}
	}

	if (in_server_block) {
		_servers.push_back(current_server);
	}

	return _servers;
}

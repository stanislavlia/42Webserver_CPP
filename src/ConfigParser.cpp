/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 11:52:22 by moetienn          #+#    #+#             */
/*   Updated: 2024/09/10 13:04:43 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

// CANONICAL FORM

ConfigParser::ConfigParser(const std::string& configFile) : _configFile(configFile)
{
	std::cout << "ConfigParser constructor" << std::endl;
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
	std::cout << "ConfigParser destructor" << std::endl;
}

// HELPERS FUNCTIONS

void    parseListen(std::istringstream& iss, ServerParam& server)
{
	int listen;
	std::cout << "Parsing listen" << iss.str() << std::endl;
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

void    parseIndex(std::istringstream& iss, ServerParam& server)
{
	std::string index;
	std::getline(iss, index, ';');
	iss >> index;
	server.setIndex(index);
}


void    parseAutoIndex(std::istringstream& iss, ServerParam& server)
{
	std::string line;
	std::getline(iss, line, ';');
	line = line.substr(1);
	std::cout << "Autoindex Before setting : " << line << std::endl;

	if (line == "on")
	{
		server.setAutoIndex(true);
	}
	else if (line == "off")
	{
		std::cout << "Autoindex is off" << std::endl;
		server.setAutoIndex(false);
	}
	else
	{
		std::cerr << "Invalid autoindex value: " << line << std::endl;
		server.setAutoIndex(false);
	}
	std::cout << "Autoindex with Getter inside ParseAutoIndex: " << server.getAutoIndex() << std::endl;
}

void    parseRoot(std::istringstream& iss, ServerParam& server)
{
	std::string root;
	std::getline(iss, root, ';');
	root = root.substr(1);
	std::cout << "Root Before setting : " << root << std::endl;
	server.setRoot(root);
	std::cout << "Root with Getter inside ParseRoot: " << server.getRoot() << std::endl;
}

void    parseAllowedMethods(std::istringstream& iss, ServerParam& server)
{
	std::string methods;
	std::getline(iss, methods, ';');
	methods = methods.substr(1);
	std::cout << "Methods Before setting : " << methods << std::endl;
	std::vector<std::string> allowed_methods;
	std::string method;
	std::istringstream iss_methods(methods);
	while (std::getline(iss_methods, method, ' '))
	{
		allowed_methods.push_back(method);
	}
	server.setAllowedMethods(allowed_methods);
	std::cout << "Methods with Getter inside ParseAllowedMethods: " << server.getAllowedMethods().size() << std::endl;
}

void	parseErrorPage(std::istringstream& iss, ServerParam& server)
{
	std::string error_code_str;
	std::string error_path;

	iss >> error_code_str >> error_path;

	int error_code = std::atoi(error_code_str.c_str());

	server.setErrorPage(error_code, error_path);
}

void	parseClientMaxBodySize(std::istringstream& iss, ServerParam& server)
{
	int clientMaxBodySize;
	iss >> clientMaxBodySize;
	server.setClientMaxBodySize(clientMaxBodySize);
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
 * - }
 * 
 *  @return std::vector<ServerParam> servers
 */

std::vector<ServerParam>    ConfigParser::parse()
{
	std::vector<ServerParam> servers;
	
	std::ifstream file(_configFile.c_str());

	if (!file.is_open())
	{
		throw std::runtime_error("Unable to open config file: " + _configFile);
	}
	
	std::string tokens[TOKEN_COUNT] = {"listen", "server_name", "client_max_body_size" , "index", "error_page", "location", "autoindex", "root", "allowed_methods" , "server", "}"};
	void (*functions[TOKEN_COUNT])(std::istringstream&, ServerParam&) = {parseListen, parseServerName, parseIndex, parseErrorPage, parseAutoIndex, parseRoot, parseAllowedMethods, parseClientMaxBodySize};
	

	std::string line;
	ServerParam current_server;
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
					case CLIENT_MAX_BODY_SIZE:
						functions[7](iss, current_server);
						break;
					case INDEX:
						if (in_location_block)
							functions[2](iss, current_server);
						break;
					case ERROR_PAGE:
						functions[3](iss, current_server);
						break;
					case LOCATION:
						in_location_block = true;
						break;
					case AUTOINDEX:
						functions[4](iss, current_server);
						break;
					case ROOT:
						functions[5](iss, current_server);
						break;
					case ALLOWED_METHODS:
						functions[6](iss, current_server);
						break;
					case SERVER:
						if (in_server_block)
						{
							servers.push_back(current_server);
							current_server = ServerParam();
						}
						in_server_block = true;
						break;
					case BRACKET:
						if (in_location_block)
							in_location_block = false;
						else if (in_server_block)
						{
							servers.push_back(current_server);
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
		servers.push_back(current_server);
	}

	return servers;
}
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 11:52:22 by moetienn          #+#    #+#             */
/*   Updated: 2024/09/09 13:05:08 by moetienn         ###   ########.fr       */
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

std::vector<ServerParam> ConfigParser::parse()
{
    std::vector<ServerParam> servers;
    std::ifstream file(_configFile.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open config file: " + _configFile);
    }

    std::string line;
    ServerParam current_server;
    bool in_server_block = false;
    bool in_location_block = false;

    while (std::getline(file, line))
	{
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "server")
		{
            if (in_server_block)
			{
                servers.push_back(current_server);
                current_server = ServerParam();
            }
            in_server_block = true;
        }
		else if (token == "listen")
		{
            int listen;
            iss >> listen;
            current_server.setListen(listen);
        }
		else if (token == "server_name")
		{
            std::string server_name;
            std::getline(iss, server_name, ';');
            server_name = server_name.substr(2); // Remove leading ": "
            current_server.setServerName(server_name);
        }
		else if(token == "location")
		{
            in_location_block = true;
        }
		else if (token == "index" && in_location_block)
		{
            std::string index;
            iss >> index;
            current_server.setIndex(index);
        }
		else if (token == "error_page")
		{
            std::string error_page;
            iss >> error_page;
            current_server.setErrorPage(error_page);
        }
		else if (token == "}")
		{
            if (in_location_block)
			{
                in_location_block = false;
            }
			else if (in_server_block)
			{
                servers.push_back(current_server);
                in_server_block = false;
            }
        }
    }

    if (in_server_block)
	{
        servers.push_back(current_server);
    }

    return (servers);
}
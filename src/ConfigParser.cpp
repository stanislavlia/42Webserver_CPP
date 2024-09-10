/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 11:52:22 by moetienn          #+#    #+#             */
/*   Updated: 2024/09/10 09:29:37 by moetienn         ###   ########.fr       */
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

void    parseErrorPage(std::istringstream& iss, ServerParam& server)
{
    std::string error_page;
    iss >> error_page;
    std::cout << "Error page: ";
    std::cout << error_page << std::endl;
    server.setErrorPage(404, error_page);
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
    
    std::string tokens[] = {"listen", "server_name", "index", "error_page", "location", "autoindex", "root", "server", "}"};
    void (*functions[])(std::istringstream&, ServerParam&) = {parseListen, parseServerName, parseIndex, parseErrorPage, parseAutoIndex, parseRoot};

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
        for (int i = 0; i < 9; ++i)
        {
            if (token == tokens[i])
            {
                token_found = true;
                switch (i)
                {
                    case 0:
                        functions[0](iss, current_server);
                        break;
                    case 1:
                        functions[1](iss, current_server);
                        break;
                    case 2:
                        if (in_location_block)
                            functions[2](iss, current_server);
                        break;
                    case 3:
                        functions[3](iss, current_server);
                        break;
                    case 4:
                        in_location_block = true;
                        break;
                    case 5:
                        functions[4](iss, current_server);
                        break;
                    case 6:
                        functions[5](iss, current_server);
                        break;
                    case 7:
                        if (in_server_block)
                        {
                            servers.push_back(current_server);
                            current_server = ServerParam();
                        }
                        in_server_block = true;
                        break;
                    case 8:
                        if (in_location_block)
                            in_location_block = false;
                        else if (in_server_block)
                        {
                            servers.push_back(current_server);
                            current_server = ServerParam();
                            in_server_block = false;
                        }
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
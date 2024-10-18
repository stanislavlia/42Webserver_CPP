/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerParam.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 08:17:08 by moetienn          #+#    #+#             */
/*   Updated: 2024/10/17 07:43:41 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerParam.hpp"

// CANONICAL FORM

ServerParam::ServerParam(void)
{
	this->port = 0;
	this->clientMaxBodySize = 0;
	this->locations = std::vector<Location>();
	this->host = "0.0.0.0";
	this->serverName = "";
	this->root = "";
	this->index = "";
	this->autoIndex = false;
	this->errorPage[0] = ""; 
}

ServerParam::ServerParam(ServerParam const &src)
{
	*this = src;
}


ServerParam &ServerParam::operator=(ServerParam const &rhs)
{
	if (this != &rhs)
	{
		this->port = rhs.port;
		this->clientMaxBodySize = rhs.clientMaxBodySize;
		this->locations = rhs.locations;
		this->host = rhs.host;
		this->serverName = rhs.serverName;
		this->index = rhs.index;
		this->errorPage = rhs.errorPage;
		this->root = rhs.root;
		this->autoIndex = rhs.autoIndex;
		this->allowedMethods = rhs.allowedMethods;
	}
	return *this;
}

ServerParam::~ServerParam(void)
{
}

// END OF CANONICAL FORM

// SETTERS

void	ServerParam::setListen(int port)
{
	this->port = port;
}

void	ServerParam::setServerName(std::string serverName)
{
	this->serverName = serverName;
}

void	ServerParam::setIndex(std::string index)
{
	this->index = index;
}

void	ServerParam::setErrorPage(int errorValue, std::string errorPage)
{
	this->errorPage[errorValue] = errorPage;
}

void	ServerParam::setRoot(std::string root)
{
	this->root = root;
}

void	ServerParam::setAutoIndex(bool autoIndex)
{
	this->autoIndex = autoIndex;
}

void	ServerParam::setAllowedMethods(std::vector<std::string> allowedMethods)
{
	this->allowedMethods = allowedMethods;
}

void	ServerParam::setClientMaxBodySize(int clientMaxBodySize)
{
	this->clientMaxBodySize = clientMaxBodySize;
}

void	ServerParam::setHost(const char *host)
{
	this->host = host;
}

// void	ServerParam::setErrorPage(std::map<int, std::string> errorPage)
// {
// 	this->errorPage = errorPage;
// }

// END OF SETTERS

// GETTERS

int ServerParam::getPort(void) const
{
	return this->port;
}

std::string	ServerParam::getServerName(void) const
{
	return this->serverName;
}

// std::string ServerParam::getIndex(void) const
// {
// 	return this->index;
// }

std::map<int, std::string> ServerParam::getErrorPage(void) const
{
	return this->errorPage;
}

std::string ServerParam::getRoot(void) const
{
	return this->root;
}

bool ServerParam::getAutoIndex(void) const
{
	return this->autoIndex;
}

std::vector<std::string> ServerParam::getAllowedMethods(void) const
{
	return this->allowedMethods;
}

int	ServerParam::getClientMaxBodySize(void) const
{
	return this->clientMaxBodySize;
}

std::string ServerParam::getHost(void) const
{
	return this->host;
}

std::vector<Location> ServerParam::getLocations(void) const
{
	return this->locations;
}

// END OF GETTERS

void	ServerParam::addLocation(Location location)
{
	this->locations.push_back(location);
}
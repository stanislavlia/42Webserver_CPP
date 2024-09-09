/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerParam.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 08:17:08 by moetienn          #+#    #+#             */
/*   Updated: 2024/09/09 13:02:46 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerParam.hpp"

// CANONICAL FORM

ServerParam::ServerParam(void)
{
	this->port = 0;
	this->serverName = "";
	this->index = "";
	this->errorPage = "";
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
		this->serverName = rhs.serverName;
		this->index = rhs.index;
		this->errorPage = rhs.errorPage;
	}
	return *this;
}



ServerParam::~ServerParam(void)
{
}

// END OF CANONICAL FORM

// SETTERS

void ServerParam::setListen(int port)
{
	this->port = port;
}

void ServerParam::setServerName(std::string serverName)
{
	this->serverName = serverName;
}

void ServerParam::setIndex(std::string index)
{
	this->index = index;
}

void ServerParam::setErrorPage(std::string errorPage)
{
	this->errorPage = errorPage;
}

// END OF SETTERS

// GETTERS

int ServerParam::getPort(void) const
{
	return this->port;
}

std::string ServerParam::getServerName(void) const
{
	return this->serverName;
}

std::string ServerParam::getIndex(void) const
{
	return this->index;
}

std::string ServerParam::getErrorPage(void) const
{
	return this->errorPage;
}

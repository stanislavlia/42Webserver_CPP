/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 03:29:13 by moetienn          #+#    #+#             */
/*   Updated: 2024/12/30 14:46:20 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

// CANONICAL FORM

Location::Location(void)
{
	this->location_name = "";
	this->clientMaxBodySize = 0;
	this->cgi_path = "";
	this->index = "";
	this->root = "";
	this->allowedMethods = std::vector<std::string>();
	this->autoIndex = false;
	this->errorPage = std::map<int, std::string>();
}

Location::Location(Location const &src)
{
	*this = src;
}

Location &Location::operator=(Location const &rhs)
{
	if (this != &rhs)
	{
		this->location_name = rhs.location_name;
		this->clientMaxBodySize = rhs.clientMaxBodySize;
		this->cgi_path = rhs.cgi_path;
		this->index = rhs.index;
		this->root = rhs.root;
		this->allowedMethods = rhs.allowedMethods;
		this->autoIndex = rhs.autoIndex;
		this->errorPage = rhs.errorPage;
	}
	return (*this);
}

Location::~Location(void)
{
}

// END OF CANONICAL FORM

// SETTERS

void	Location::setLocationName(std::string location_name)
{
	this->location_name = location_name;
}

void	Location::setIndex(std::string index)
{
	this->index = index;
}

void	Location::setErrorPage(int errorValue, std::string errorPage)
{
	this->errorPage[errorValue] = errorPage;
}

void	Location::setRoot(std::string root)
{
	this->root = root;
}

void	Location::setAutoIndex(bool autoIndex)
{
	this->autoIndex = autoIndex;
}

void	Location::setAllowedMethods(std::vector<std::string> allowedMethods)
{
	this->allowedMethods = allowedMethods;
}

void	Location::setErrorPage(std::map<int, std::string> errorPage)
{
	this->errorPage = errorPage;
}

void	Location::setClientMaxBodySize(int clientMaxBodySize)
{
	this->clientMaxBodySize = clientMaxBodySize;
}

void	Location::setCgiPath(std::string cgi_path)
{
	this->cgi_path = cgi_path;
}

// END OF SETTERS

// GETTERS

std::string	Location::getLocationName(void) const
{
	return (this->location_name);
}

std::string	Location::getIndex(void) const
{
	return (this->index);
}

std::string	Location::getRoot(void) const
{
	return (this->root);
}

bool	Location::getAutoIndex(void) const
{
	return (this->autoIndex);
}

std::vector<std::string>	Location::getAllowedMethods(void) const
{
	return (this->allowedMethods);
}

std::map<int, std::string>	Location::getErrorPage(void) const
{
	return (this->errorPage);
}

int	Location::getClientMaxBodySize(void) const
{
	return (this->clientMaxBodySize);
}

std::string	Location::getCgiPath(void) const
{
	return (this->cgi_path);
}
// END OF GETTERS

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 03:29:13 by moetienn          #+#    #+#             */
/*   Updated: 2024/10/17 03:32:58 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

// CANONICAL FORM

Location::Location(void)
{
	this->clientMaxBodySize = 0;
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
		this->clientMaxBodySize = rhs.clientMaxBodySize;
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

// END OF SETTERS

// GETTERS

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

// END OF GETTERS
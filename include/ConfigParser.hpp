/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 11:52:19 by moetienn          #+#    #+#             */
/*   Updated: 2024/10/17 03:34:50 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "Webserver.hpp"
#include "ServerParam.hpp"
#include "Location.hpp"

enum FunctionType {
	LISTEN,
	SERVER_NAME,
	HOST_NAME,
	CLIENT_MAX_BODY_SIZE,
	INDEX,
	ERROR_PAGE,
	LOCATION,
	AUTOINDEX,
	ROOT,
	ALLOWED_METHODS,
	SERVER,
	BRACKET,
	UNKNOWN,
	TOKEN_COUNT
}	;

class ConfigParser
{
	private:
		std::string _configFile;
		std::vector<ServerParam> _servers;

	public:
		ConfigParser(const std::string& configFile);
		ConfigParser(ConfigParser const &src);
		ConfigParser &operator=(ConfigParser const &rhs);
		~ConfigParser(void);
		
		
		std::vector<ServerParam> parse(void);
}	;

#endif
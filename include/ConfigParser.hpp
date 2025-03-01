/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 11:52:19 by moetienn          #+#    #+#             */
/*   Updated: 2025/01/28 05:02:47 by marvin           ###   ########.fr       */
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
	CGI_PATH,
	AUTOINDEX,
	ROOT,
	ALLOWED_METHODS,
	SERVER,
	RETURN,
	CGI_EXTENSION,
	CGI_INTERPRETER,
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
		
		std::vector<ServerParam>	parse(void);
}	;

#endif
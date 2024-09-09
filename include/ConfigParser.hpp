/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 11:52:19 by moetienn          #+#    #+#             */
/*   Updated: 2024/09/09 12:59:28 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "Webserver.hpp"
#include "ServerParam.hpp"

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
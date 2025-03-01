/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerParam.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 08:13:22 by moetienn          #+#    #+#             */
/*   Updated: 2025/01/06 12:29:30 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERPARAM_HPP
#define SERVERPARAM_HPP

#include "Webserver.hpp"
#include "Location.hpp"

class ServerParam
{
	private:
		int									port;
		std::string							host;
		int									clientMaxBodySize;
		std::string							serverName;
		std::string							index;
		std::string							root;
		std::vector<std::string>			allowedMethods;
		bool								autoIndex;
		std::map<int, std::string>			errorPage;
		
	public:
		ServerParam(void);
		ServerParam(ServerParam const &src);
		ServerParam &operator=(ServerParam const &rhs);
		~ServerParam(void);

		std::vector<Location>				locations;
		void	setListen(int port);
		void	setServerName(std::string serverName);
		void	setIndex(std::string index);
		void	setErrorPage(int errorValue, std::string errorPage);
		void	setRoot(std::string root);
		void	setAutoIndex(bool autoIndex);
		void	setAllowedMethods(std::vector<std::string> allowedMethods);
		void	setHost(const char *host);
		void	setClientMaxBodySize(int clientMaxBodySize);


		int							getPort(void) const;
		std::string					getServerName(void) const;
		std::string					getRoot(void) const;
		bool						getAutoIndex(void) const;
		std::vector<std::string>	getAllowedMethods(void) const;
		std::map<int, std::string>	getErrorPage(void) const;
		int							getClientMaxBodySize(void) const;
		std::string					getHost(void) const;

		void						addLocation(Location location);
		std::vector<Location>		getLocations(void) const;
}	;

#endif
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerParam.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 08:13:22 by moetienn          #+#    #+#             */
/*   Updated: 2024/09/10 08:16:31 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERPARAM_HPP
#define SERVERPARAM_HPP

#include "Webserver.hpp"

class ServerParam
{
	private:
		int									port;
		std::string							serverName;
		std::string							index;
		std::string							root;
		std::map<std::string, std::string>	allowedMethods;
		bool								autoIndex;
		std::map<int, std::string>			errorPage;
		
	public:
		ServerParam(void);
		ServerParam(ServerParam const &src);
		ServerParam &operator=(ServerParam const &rhs);
		~ServerParam(void);

		void	setListen(int port);
		void	setServerName(std::string serverName);
		void	setIndex(std::string index);
		void	setErrorPage(int errorValue, std::string errorPage);
		void	setRoot(std::string root);
		void	setAutoIndex(bool autoIndex);


		int							getPort(void) const;
		std::string					getServerName(void) const;
		std::string					getIndex(void) const;
		std::string					getRoot(void) const;
		std::map<int, std::string>	getErrorPage(void) const;
		bool						getAutoIndex(void) const;
	
}	;

#endif
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerParam.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 08:13:22 by moetienn          #+#    #+#             */
/*   Updated: 2024/09/09 13:03:12 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERPARAM_HPP
#define SERVERPARAM_HPP

#include "Webserver.hpp"

class ServerParam
{
	private:
		int	port;
		std::string serverName;
		std::string index;
		std::string errorPage;
	public:
		ServerParam(void);
		ServerParam(ServerParam const &src);
		ServerParam &operator=(ServerParam const &rhs);
		~ServerParam(void);

		void setListen(int port);
		void setServerName(std::string serverName);
		void setIndex(std::string index);
		void setErrorPage(std::string errorPage);


		int getPort(void) const;
		std::string getServerName(void) const;
		std::string getIndex(void) const;
		std::string getErrorPage(void) const;
		
	
}	;

#endif
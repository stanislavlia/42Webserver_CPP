/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 03:25:34 by moetienn          #+#    #+#             */
/*   Updated: 2024/10/17 06:39:30 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Webserver.hpp"

class Location
{
	private:
		int									clientMaxBodySize;
		std::string							index;
		std::string							root;
		std::vector<std::string>			allowedMethods;
		bool								autoIndex;
		std::map<int, std::string>			errorPage;
	
	public:
		Location(void);
		Location(Location const &src);
		Location &operator=(Location const &rhs);
		~Location(void);

		// SETTERS

		void	setIndex(std::string index);
		void	setErrorPage(int errorValue, std::string errorPage);
		void	setRoot(std::string root);
		void	setAutoIndex(bool autoIndex);
		void	setAllowedMethods(std::vector<std::string> allowedMethods);
		void	setErrorPage(std::map<int, std::string> errorPage);
		void	setClientMaxBodySize(int clientMaxBodySize);
		
		// GETTERS
		
		std::string					getIndex(void) const;
		std::string					getRoot(void) const;
		bool						getAutoIndex(void) const;
		std::vector<std::string>	getAllowedMethods(void) const;
		std::map<int, std::string>	getErrorPage(void) const;
		int							getClientMaxBodySize(void) const;
}	;

#endif

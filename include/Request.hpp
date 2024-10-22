/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/11 07:10:22 by moetienn          #+#    #+#             */
/*   Updated: 2024/10/22 11:22:24 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Webserver.hpp"
# include "ServerParam.hpp"
# include <algorithm>

class	Request
{
	private:
		const ServerParam& _config;

		std::string _method;
		std::string _uri;
		std::map <std::string, std::string> _headers;
		std::string _body;
		int			_valid;

		void    parseHeaders(const std::string& headers);
		void    parseBody(const std::string& body);

		void		validateRequest();
	public:
		Request(const ServerParam& config);
		Request(const Request& src);
		Request& operator=(const Request& rhs);
		~Request();

		void    parseRequest(const std::string& request);

		std::string	getMethod() const;
		std::string	getUri() const;
		std::map<std::string, std::string>	getHeaders() const;
		std::string	getBody() const;
		int			isValid() const;

		std::vector<std::string> split(const std::string& str, const std::string& delimiter);
}	;


#endif
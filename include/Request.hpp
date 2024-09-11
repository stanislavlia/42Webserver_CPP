/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/11 07:10:22 by moetienn          #+#    #+#             */
/*   Updated: 2024/09/11 09:01:12 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Webserver.hpp"

class	Request
{
	private:
		std::string _method;
		std::string _uri;
		std::string _headers;
		std::string _body;
	public:
		Request();
		Request(const Request& src);
		Request& operator=(const Request& rhs);
		~Request();

		void    parseHeaders(const std::string& headers);
		void    parseBody(const std::string& body);
		void    parseRequest(const std::string& request);

		std::string	getMethod() const;
		std::string	getUri() const;
		std::string	getHeaders() const;
		std::string	getBody() const;
}	;


#endif
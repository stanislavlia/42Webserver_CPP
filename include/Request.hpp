/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/11 07:10:22 by moetienn          #+#    #+#             */
/*   Updated: 2025/01/20 12:34:32 by marvin           ###   ########.fr       */
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
		std::vector<char> _body;
		int			_valid;

		void    parseHeaders(const std::string& headers);
		void    parseBody(const std::vector<char>& body);

		void		validateRequest();
	public:
		Request(const ServerParam& config);
		Request(const Request& src);
		Request& operator=(const Request& rhs);
		~Request();

		void    parseRequest(const std::string& request);

		void	addHeader(const std::string& key, const std::string& value);
		std::string	getMethod() const;
		std::string	getUri() const;
		std::map<std::string, std::string>	getHeaders() const;
		std::vector<char>	getBody() const;
		void	setBody(const std::vector<char>& body);
		int			isValid() const;

		std::vector<std::string> split(const std::string& str, const std::string& delimiter);
}	;


#endif
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/11 07:10:19 by moetienn          #+#    #+#             */
/*   Updated: 2024/09/11 09:00:54 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

// CANONICAL FORM

Request::Request()
{
}

Request::Request(const Request& src)
{
	*this = src;
}

Request& Request::operator=(const Request& rhs)
{
	if (this != &rhs)
	{
		_method = rhs._method;
		_uri = rhs._uri;
		_headers = rhs._headers;
		_body = rhs._body;
	}
	return *this;
}

Request::~Request()
{
}

// END CANONICAL FORM

// GETTERS

std::string	Request::getMethod() const
{
	return _method;
}

std::string	Request::getUri() const
{
	return _uri;
}

std::string	Request::getHeaders() const
{
	return _headers;
}

std::string	Request::getBody() const
{
	return _body;
}

// PARSERS

std::vector<std::string> split(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start, end));
    return tokens;
}

void	Request::parseHeaders(const std::string& headers)
{
	_headers = headers;
}

void	Request::parseBody(const std::string& body)
{
	_body = body;
}

void Request::parseRequest(const std::string& rawRequest)
{
    std::istringstream requestStream(rawRequest);
    std::string line;

    // Parse request line
    if (std::getline(requestStream, line))
	{
        std::istringstream lineStream(line);
        lineStream >> _method;
        lineStream >> _uri;
    }

    // Parse headers
    std::string headersStr;
    while (std::getline(requestStream, line) && line != "\r")
	{
        headersStr += line + "\n";
    }
    parseHeaders(headersStr);

    // Parse body
    std::string bodyStr;
    while (std::getline(requestStream, line))
	{
        bodyStr += line + "\n";
    }
    parseBody(bodyStr);
}


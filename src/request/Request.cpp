/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/11 07:10:19 by moetienn          #+#    #+#             */
/*   Updated: 2024/09/20 11:24:42 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

// CANONICAL FORM

Request::Request(const ServerParam& config)
: _config(config), _method(""), _uri(""), _headers(""), _body(""), _valid(false)
{
}

Request::Request(const Request& src)
: _config(src._config), _method(src._method), _uri(src._uri), _headers(src._headers), _body(src._body), _valid(src._valid)
{
}

Request& Request::operator=(const Request& rhs)
{
    if (this != &rhs)
    {
        _method = rhs._method;
        _uri = rhs._uri;
        _headers = rhs._headers;
        _body = rhs._body;
        _valid = rhs._valid;
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

int    Request::isValid() const
{
    return _valid;
}

// PARSERS

void	Request::validateRequest()
{
    if (_method.empty())
    {
        _valid = 1;
        return ;
    }
    else if (_uri.empty())
    {
        _valid = 3;
        return ;
    }
    else if (_config.getAllowedMethods().size() > 0)
    {
        for (size_t i = 0; i < _config.getAllowedMethods().size(); i++)
        {
            if (_method == _config.getAllowedMethods().at(i))
            {
                break ;
            }
            else if (i == _config.getAllowedMethods().size() - 1)
            {
                _valid = 2;
                return ;
            }
        }
    }
    _valid = 0;
}

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
    validateRequest();
}


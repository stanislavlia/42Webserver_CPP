/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/11 07:10:19 by moetienn          #+#    #+#             */
/*   Updated: 2024/12/13 14:23:33 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

// CANONICAL FORM

Request::Request(const ServerParam& config)
: _config(config), _method(""), _uri(""), _headers(), _body(""), _valid(false)
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

std::map<std::string, std::string>	Request::getHeaders() const
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
    else if (_config.locations[0].getAllowedMethods().size() > 0)
    {
        for (size_t i = 0; i < _config.locations[0].getAllowedMethods().size(); i++)
        {
            if (_method == _config.locations[0].getAllowedMethods().at(i))
            {
                break ;
            }
            else if (i == _config.locations[0].getAllowedMethods().size() - 1)
            {
                _valid = 2;
                return ;
            }
        }
    }
    _valid = 0;
}

std::vector<std::string>    Request::split(const std::string& str, const std::string& delimiter)
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
    // std::cout << "In parseHeaders" << std::endl;
    std::vector<std::string> headersList = split(headers, "\n");
    for (size_t i = 0; i < headersList.size(); i++)
    {
        // std::cout << "headersList.at(i): " << headersList.at(i) << std::endl;
        std::vector<std::string> header = split(headersList.at(i), ": ");
        if (header.size() == 2)
        {
            _headers[header.at(0)] = header.at(1);
        }
    }
}

void	Request::parseBody(const std::string& body)
{
	_body = body;
}

void    Request::parseRequest(const std::string& rawRequest)
{
    std::istringstream requestStream(rawRequest);
    std::string line;

    // std::cout << "===== RAW REQUEST SIZE =====" << rawRequest.size() << std::endl;
    // Parse request line
    if (std::getline(requestStream, line))
    {
        std::istringstream lineStream(line);
        lineStream >> _method;
        lineStream >> _uri;
    }
    // Parse headers
    std::string headersStr;
    std::map<std::string, std::string> headers;
    while (std::getline(requestStream, line) && line != "\r")
    {
        headersStr += line + "\n";
        std::istringstream headerStream(line);
        std::string key, value;
        if (std::getline(headerStream, key, ':') && std::getline(headerStream, value))
        {
            // Remove leading spaces from value
            value.erase(0, value.find_first_not_of(" \t"));
            headers[key] = value;
        }
    }
    parseHeaders(headersStr);

    // Parse body
    if (_method == "POST")
    {
        std::map<std::string, std::string>::iterator contentLengthIt = headers.find("Content-Length");
        if (contentLengthIt != headers.end())
        {
            int contentLength = std::atoi(contentLengthIt->second.c_str());
            std::vector<char> body(contentLength);
            requestStream.read(&body[0], contentLength);
            std::string bodyStr(body.begin(), body.end());
            parseBody(bodyStr);
            // std::cout << "==== BODY SIZE IN PARSE REQUEST: " << _body.size() << std::endl;
            // std::cout << "==== BITS BODY SIZE IN PARSE REQUEST: " << body.size() << std::endl;
        }
    }
    validateRequest();
}

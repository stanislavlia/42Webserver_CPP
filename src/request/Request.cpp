/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/11 07:10:19 by moetienn          #+#    #+#             */
/*   Updated: 2025/01/28 06:37:19 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

// CANONICAL FORM

Request::Request(const ServerParam& config)
: _config(config), _method(""), _uri(""), _headers(), _body(), _valid(false)
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

void    Request::setBody(const std::vector<char>& body)
{
    _body = body;
}

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


std::vector<char>	Request::getBody() const
{
    return _body;
}

int    Request::isValid() const
{
    return _valid;
}

// PARSERS

void	Request::addHeader(const std::string& key, const std::string& value)
{
    _headers[key] = value;
}

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
    if (_headers.find("Content-Length") != _headers.end())
    {
        std::istringstream iss(_headers.at("Content-Length"));
        int content_length;
        iss >> content_length;

        if (content_length > _config.getClientMaxBodySize() * 1024 * 1024)
        {
            _valid = 4;
            return ;
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
    std::vector<std::string> headersList = split(headers, "\n");
    for (size_t i = 0; i < headersList.size(); i++)
    {
        std::vector<std::string> header = split(headersList.at(i), ": ");
        if (header.size() == 2)
        {
            _headers[header.at(0)] = header.at(1);
        }
    }
}

void	Request::parseBody(const std::vector<char>& body)
{
    _body = body;
}

#include <sstream>

void restoreBody(std::vector<char>& body)
{
    std::vector<char> restoredBody;
    size_t pos = 0;

    while (pos < body.size())
    {
        // Find the position of the next chunk size
        std::vector<char>::iterator chunkSizeEndIt = std::search(body.begin() + pos, body.end(), "\r\n", "\r\n" + 2);
        if (chunkSizeEndIt == body.end())
            break;

        // Get the chunk size as a string and convert it to an integer
        std::string chunkSizeStr(body.begin() + pos, chunkSizeEndIt);
        std::stringstream ss;
        ss << std::hex << chunkSizeStr;
        size_t chunkSize;
        ss >> chunkSize;

        // Move the position to the start of the chunk data
        pos = std::distance(body.begin(), chunkSizeEndIt) + 2;

        // Append the chunk data to the restored body
        restoredBody.insert(restoredBody.end(), body.begin() + pos, body.begin() + pos + chunkSize);

        // Move the position to the start of the next chunk size
        pos += chunkSize + 2;
    }

    body = restoredBody;
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
        std::istringstream headerStream(line);
        std::string key, value;
        if (std::getline(headerStream, key, ':') && std::getline(headerStream, value))
        {
            // Remove leading spaces from value
            value.erase(0, value.find_first_not_of(" \t"));
            _headers[key] = value;
        }
    }
    parseHeaders(headersStr);

    if (_method == "POST")
    {
        std::map<std::string, std::string>::iterator contentLengthIt = _headers.find("Content-Length");
        std::map<std::string, std::string>::iterator transferEncodingIt = _headers.find("Transfer-Encoding");
        
        if (contentLengthIt != _headers.end())
        {
            std::vector<char> body(rawRequest.begin() + rawRequest.find("\r\n\r\n") + 4, rawRequest.end());
            
            requestStream.read(&body[0], rawRequest.size() - rawRequest.find("\r\n\r\n") - 4);
            
            parseBody(body);
        }
        else if (transferEncodingIt != _headers.end() && transferEncodingIt->second.find("chunked\r") != std::string::npos)
        {
            std::vector<char> body;
            std::string line;
    
            while (true)
            {
                // Read the chunk size line
                std::getline(requestStream, line);
    
                // Strip any extraneous whitespace or CRLF characters
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
                line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
    
                // Convert the chunk size from hexadecimal to an integer
                std::istringstream chunkSizeStream(line);
                int chunkSize;
                chunkSizeStream >> std::hex >> chunkSize;
    
                // If chunk size is 0, the transfer is done
                if (chunkSize == 0)
                {
                    break;
                }
    
                // Read the chunk data
                std::vector<char> chunk(chunkSize);
                requestStream.read(chunk.data(), chunkSize);
                if (requestStream.gcount() != chunkSize)
                {
                    std::cerr << "Error reading chunk data" << std::endl;
                    break;
                }
    
                // Append chunk to body
                body.insert(body.end(), chunk.begin(), chunk.end());
    
                // Read the CRLF after the chunk
                std::getline(requestStream, line);
            }
            
            restoreBody(body);
            parseBody(body);
        }
    }
    validateRequest();
}
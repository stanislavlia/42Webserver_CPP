/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 06:56:33 by moetienn          #+#    #+#             */
/*   Updated: 2024/12/19 10:01:57 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"

void	RequestHandler::_handleErrorPage(int status_code, const Location& location)
{
	try 
	{
		_handleInvalidRequest(status_code, location);	
	}
	catch (const std::exception& e)
	{
		_DefaultErrorPage(status_code);
	}
}

std::string RequestHandler::_ExtractBoundaryDelimiter()
{
	size_t pos = _request.getHeaders().at("Content-Type").find("boundary=");
	if (pos == std::string::npos)
	{
		std::cerr << "Boundary delimiter not found" << std::endl;
		return "";
	}
	std::string boundary_delimiter = _request.getHeaders().at("Content-Type").substr(pos + 9);
	return boundary_delimiter;
}
std::string RequestHandler::_getExactBody(const std::string& body, int content_length)
{
    // Ensure the body has enough data
    if (body.size() < (size_t)content_length)
    {
        std::cerr << "Body is smaller than Content-Length" << std::endl;
        return "";
    }

    // Extract the exact number of bytes specified by Content-Length
    std::string exact_body = body.substr(0, content_length);
    return exact_body;
}

void	RequestHandler::_ParseMultipartFormData(const std::string& body, const std::string& boundary_delimiter, const Location& location)
{
	// Split the body into parts based on the boundary delimiter\n";
	std::vector<std::string> parts = _request.split(body, "--" + boundary_delimiter);
	//print the parts

	for (size_t i = 0; i < parts.size(); i++)
	{
		if (parts[i].empty() || parts[i] == "--")
		{
			continue;
		}
		size_t headerEnd = parts[i].find("\r\n\r\n");
		if (headerEnd == std::string::npos)
		{
			continue;
		}
		std::string	headers = parts[i].substr(0, headerEnd);
		std::string	content = parts[i].substr(headerEnd + 4);

		size_t contentEnd = content.find("\r\n--" + boundary_delimiter);
        if (contentEnd == std::string::npos)
		{
			contentEnd = content.find("\r\n--");
            content = content.substr(0, contentEnd);
        }

		std::istringstream	iss(headers);
		std::string			line;
		
		std::map<std::string, std::string> headersMap;
		while (std::getline(iss, line))
		{
			size_t colonPos = line.find(": ");
			if (colonPos != std::string::npos)
			{
				std::string	headerName = line.substr(0, colonPos);
				std::string headerValue = line.substr(colonPos + 2);
				headersMap[headerName] = headerValue;
			}
		}
		if (headersMap.find("Content-Disposition") != headersMap.end())
		{
			std::string	contentDisposition = headersMap["Content-Disposition"];
			size_t		fileNamePos = contentDisposition.find("filename=\"");
			if (fileNamePos != std::string::npos)
			{
				size_t		fileNameEnd = contentDisposition.find("\"", fileNamePos + 10);
				std::string	fileName = contentDisposition.substr(fileNamePos + 10, fileNameEnd - fileNamePos - 10);

				std::string	filePath = location.getRoot() + "/" + fileName;
				std::ofstream	outfile(filePath.c_str());
				if (outfile)
				{
					std::cout << "Writing to file: " << filePath << std::endl;
					outfile.write(content.c_str(), content.size());
					outfile.close();
					std::cout << "File uploaded successfully" << std::endl;
					_serveHtmlContent("<h1>File uploaded successfully</h1>", 200, "OK");
				}
				else
				{
					std::cerr << "Error opening file for writing: " << fileName << std::endl;
				}
			}
		}
	}
}

void	RequestHandler::_handlePostRequest(const std::string& rootDir, const Location& location)
{
	std::istringstream iss(_request.getHeaders().at("Content-Length"));
	int	content_length;
	iss >> content_length;

	(void)rootDir;
	// check the content length header exists
	if (_request.getHeaders().find("Content-Length") == _request.getHeaders().end())
	{
		try 
		{
			_handleInvalidRequest(411, location);	
		}
		catch (const std::exception& e)
		{
			_DefaultErrorPage(411);
		}
	}
	// check if the content length is within acceptable limits
	else if (content_length > _config.getClientMaxBodySize())
	{
		try 
		{
			_handleInvalidRequest(413, location);	
		}
		catch (const std::exception& e)
		{
			_DefaultErrorPage(413);
		}
	}
	// check the content type header exists
	else if (_request.getHeaders().find("Content-Type") == _request.getHeaders().end())
	{
		try 
		{
			_handleInvalidRequest(400, location);	
		}
		catch (const std::exception& e)
		{
			_DefaultErrorPage(400);
		}
	}
	if (_request.getHeaders().at("Content-Type").find("multipart/form-data") != std::string::npos)
	{
		std::string	boundary_delimiter =_ExtractBoundaryDelimiter();
		std::string body = _getExactBody(_request.getBody(), content_length);

		std::cout << "Parse multipart form data" << std::endl;
		_ParseMultipartFormData(body, boundary_delimiter, location);
	}
}
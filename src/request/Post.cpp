/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 06:56:33 by moetienn          #+#    #+#             */
/*   Updated: 2024/12/13 12:00:44 by marvin           ###   ########.fr       */
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
	std::cout << "body size: " << body.size() << std::endl;
	std::cout << "content length: " << content_length << std::endl;
    if (body.size() < (size_t)content_length)
    {
        std::cerr << "Body is smaller than Content-Length" << std::endl;
        return "";
    }

    // Extract the exact number of bytes specified by Content-Length
    std::string exact_body = body.substr(0, content_length);
	std::cout << "exact body: " << exact_body << std::endl;
    return exact_body;
}

void	RequestHandler::_ParseMultipartFormData(const std::string& body, const std::string& boundary_delimiter, const Location& location)
{
	std::cout << "==== IN MULTIPART FORM DATA 2 ====" << std::endl;
	// Split the body into parts based on the boundary delimiter\n";
	std::vector<std::string> parts = _request.split(body, "--" + boundary_delimiter);
	//print the parts

	std::cout << "parts size: " << parts.size() << std::endl;
	
	for (size_t i = 0; i < parts.size(); i++)
	{
		std::cout << "==== I ====" << i << std::endl;
		if (parts[i].empty() || parts[i] == "--")
		{
			std::cout << "Empty part" << std::endl;  
			continue;
		}
		size_t headerEnd = parts[i].find("\r\n\r\n");
		if (headerEnd == std::string::npos)
		{
			std::cerr << "Header end not found" << std::endl;
			continue;
		}
		std::string	headers = parts[i].substr(0, headerEnd);
		std::string	content = parts[i].substr(headerEnd + 4);

		size_t contentEnd = content.find("\r\n--" + boundary_delimiter);
		std::cout << "contentEnd: " << contentEnd << std::endl;
		std::cout << "std::string::npos: " << std::string::npos << std::endl;
        if (contentEnd == std::string::npos)
		{
			// std::cout << "In cond to remove boundary" << std::endl;
			contentEnd = content.find("\r\n--");
            content = content.substr(0, contentEnd);
			// std::cout << "content: " << content << std::endl;
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
			std ::cout << "==== Content-Disposition ==== " << contentDisposition << std::endl;
			size_t		fileNamePos = contentDisposition.find("filename=\"");
			std::cout << "==== fileNamePos ==== " << fileNamePos << std::endl;
			if (fileNamePos != std::string::npos)
			{
				size_t		fileNameEnd = contentDisposition.find("\"", fileNamePos + 10);
				std::string	fileName = contentDisposition.substr(fileNamePos + 10, fileNameEnd - fileNamePos - 10);

				std::string	filePath = location.getRoot() + "/" + fileName;
				std::ofstream	outfile(filePath.c_str());
				std::cout << "file path: " << filePath << std::endl;
				if (outfile)
				{
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
	// std::cout << "max body size: " << _config.getClientMaxBodySize() << std::endl;
	std::cout << "content type header: " << _request.getHeaders().at("Content-Type") << std::endl;
	if (_request.getHeaders().find("Content-Length") == _request.getHeaders().end())
	{
		// _handleErrorPage(411, location);
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
		// _handleErrorPage(413, location);
		std::cout << "Content-Length is too large" << std::endl;
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
		// _handleErrorPage(400, location);
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
		std::cout << "==== IN MULTIPART FORM DATA ====" << std::endl;
		// std::cout << "boundary delimiter: " << boundary_delimiter << std::endl;
		// std::cout << "body before extract: " << _request.getBody() << std::endl;
		std::string body = _getExactBody(_request.getBody(), content_length);
		// std::cout << "body: " << body << std::endl;
		std::cout << "Header " << _request.getHeaders().at("Content-Type") << std::endl;

		_ParseMultipartFormData(body, boundary_delimiter, location);
	}
	// std::cout << "BODY request: " << _request.getBody() << "End of body ------------" << std::endl;
	//read the body of the request based on the content length and parse it
	// Parse the body according to the content type
	// std::cout << "Before parsing body" << std::endl;
	// std::cout << "Content-Type: " << _request.getHeaders().at("Content-Type") << std::endl;
	// if (_request.getHeaders().at("Content-Type").find("image/png") != std::string::npos)
	// {
	// 	std::cout << "Image/png" << std::endl;
	// }
}
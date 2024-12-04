/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 06:56:33 by moetienn          #+#    #+#             */
/*   Updated: 2024/12/04 12:09:35 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"

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

void	RequestHandler::_ParseMultipartFormData(int client_fd, const std::string& body, const std::string& boundary_delimiter, const Location& location)
{
	(void)client_fd;
	// Split the body into parts based on the boundary delimiter
	std::vector<std::string> parts = _request.split(body, "--" + boundary_delimiter);
	std::cout << "parts size: " << parts.size() << std::endl;
	// std::cout << "Part 1" << parts[1] << std::endl;
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
		// std::cout << "contentEnd: " << contentEnd << std::endl;
		// std::cout << "std::string::npos: " << std::string::npos << std::endl;
        if (contentEnd == std::string::npos)
		{
			// std::cout << "In cond to remove boundary" << std::endl;
			contentEnd = content.find("\r\n");
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
			size_t		fileNamePos = contentDisposition.find("filename=\"");
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
					_serveHtmlContent(client_fd, "<h1>File uploaded successfully</h1>", 200, "OK");
				}
				else
				{
					std::cerr << "Error opening file for writing: " << fileName << std::endl;
				}
			}
		}
	}
}

void	RequestHandler::_handlePostRequest(int client_fd, const std::string& rootDir, const Location& location)
{
	std::istringstream iss(_request.getHeaders().at("Content-Length"));
	int	content_length;
	iss >> content_length;

	(void)rootDir;
	// check the content length header exists
	std::cout << "max body size: " << _config.getClientMaxBodySize() << std::endl;
	std::cout << "content type header: " << _request.getHeaders().at("Content-Type") << std::endl;
	if (_request.getHeaders().find("Content-Length") == _request.getHeaders().end())
	{
		try 
		{
			_handleInvalidRequest(client_fd, 411, location);	
		}
		catch (const std::exception& e)
		{
			_DefaultErrorPage(client_fd, 411);
		}
	}
	// check if the content length is within acceptable limits
	else if (content_length > _config.getClientMaxBodySize())
	{
		std::cout << "Content-Length is too large" << std::endl;
		try 
		{
			_handleInvalidRequest(client_fd, 413, location);	
		}
		catch (const std::exception& e)
		{
			_DefaultErrorPage(client_fd, 413);
		}
	}
	// check the content type header exists
	else if (_request.getHeaders().find("Content-Type") == _request.getHeaders().end())
	{
		try 
		{
			_handleInvalidRequest(client_fd, 400, location);	
		}
		catch (const std::exception& e)
		{
			_DefaultErrorPage(client_fd, 400);
		}
	}
	if (_request.getHeaders().at("Content-Type").find("multipart/form-data") != std::string::npos)
	{
		std::string	boundary_delimiter =_ExtractBoundaryDelimiter();
		// std::cout << "boundary delimiter: " << boundary_delimiter << std::endl;
		// std::cout << "body before extract: " << _request.getBody() << std::endl;
		std::string body = _getExactBody(_request.getBody(), content_length);
		std::cout << "body: " << body << std::endl;
		std::cout << "Header " << _request.getHeaders().at("Content-Type") << std::endl;

		_ParseMultipartFormData(client_fd, body, boundary_delimiter, location);
	}
	std::cout << "BODY request: " << _request.getBody() << "End of body ------------" << std::endl;
	//read the body of the request based on the content length and parse it
	// Parse the body according to the content type
	std::cout << "Before parsing body" << std::endl;
	std::cout << "Content-Type: " << _request.getHeaders().at("Content-Type") << std::endl;
// 	if (_request.getHeaders().at("Content-Type") == "text/plain")
// 	{
//     	// Determine the file path where the plain text should be saved
//    		std::string file_path = "uploads/" + location.getRoot() + _request.getUri(); // Update this path as needed

//     	// Open the file for writing
// 		std::cout << "file path: " << file_path << std::endl;
//     	std::ofstream outfile(file_path.c_str());
//     	if (!outfile)
//     	{
//         	std::cerr << "Error opening file for writing: " << file_path << std::endl;
//         	try
//         	{
//         	    _respond_with_error(client_fd, 500, "Internal Server Error", location);
//         	}
//         	catch (const std::exception& e)
//         	{
//         	    _DefaultErrorPage(client_fd, 500);
//         	}
//         	return;
//     	}
// 			// Write the plain text content to the file
//     		outfile << body;
//     		outfile.close();
//     		// Respond to the client indicating success
// 			_serveHtmlContent(client_fd, "<h1>File uploaded successfully</h1>", 200, "OK");
// 	}
// 	std::cout << "After parsing body" << std::endl;
}
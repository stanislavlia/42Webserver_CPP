/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 06:56:33 by moetienn          #+#    #+#             */
/*   Updated: 2025/01/28 07:03:12 by marvin           ###   ########.fr       */
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

void RequestHandler::_ParseMultipartFormData(const std::vector<char>& body, const std::string& boundary_delimiter, const Location& location, ConnectionState& connection_state)
{
    // Convert the body vector to a string to use the split function
    std::string bodyStr(body.begin(), body.end());

    std::vector<std::string> parts = _request.split(bodyStr, "--" + boundary_delimiter);

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

        std::string headers = parts[i].substr(0, headerEnd);

        // Extract binary content
        std::vector<char> content(parts[i].begin() + headerEnd + 4, parts[i].end());

        size_t contentEnd = std::string(content.begin(), content.end()).find("\r\n--" + boundary_delimiter);
        if (contentEnd != std::string::npos)
		{
            content.resize(contentEnd);
        }

        std::istringstream iss(headers);
        std::string line;

        std::map<std::string, std::string> headersMap;
        while (std::getline(iss, line))
		{
            size_t colonPos = line.find(": ");
            if (colonPos != std::string::npos)\
			{
                std::string headerName = line.substr(0, colonPos);
                std::string headerValue = line.substr(colonPos + 2);
                headersMap[headerName] = headerValue;
            }
        }

        if (headersMap.find("Content-Disposition") != headersMap.end())
		{
            std::string contentDisposition = headersMap["Content-Disposition"];
            size_t fileNamePos = contentDisposition.find("filename=\"");
            if (fileNamePos != std::string::npos)
			{
                size_t fileNameEnd = contentDisposition.find("\"", fileNamePos + 10);
                std::string fileName = contentDisposition.substr(fileNamePos + 10, fileNameEnd - fileNamePos - 10);

                std::string filePath = location.getRoot() + "/" + fileName;

				// Check if the file already exists
				struct stat buffer;
				if (stat(filePath.c_str(), &buffer) == 0 && connection_state != CHUNKED && connection_state != CHUNKED_COMPLETE)
    			{
    			    // append a number to the file name
    			    // count the number of files with the same name
    			    int count = 1;
    			    std::string newFileName;
    			    size_t dotPos = fileName.find_last_of('.');
    			    while (true)
    			    {
    			        std::stringstream ss;
    			        ss << count;
    			        if (dotPos != std::string::npos)
    			        {
    			            newFileName = fileName.substr(0, dotPos) + "(" + ss.str() + ")" + fileName.substr(dotPos);
    			        }
    			        else
    			        {
    			            newFileName = fileName + "(" + ss.str() + ")";
    			        }
    			        filePath = location.getRoot() + "/" + newFileName;
    			        if (stat(filePath.c_str(), &buffer) != 0)
    			        {
    			            break;
    			        }
    			        count++;
    			    }
    			}

				// find what has been already written in the file to no overwrite it and append the new content
				
				std::ifstream infile(filePath.c_str(), std::ios::binary);
				std::vector<char> existingContent;
				
				if (infile)
				{
				    infile.seekg(0, std::ios::end);
				    size_t fileSize = infile.tellg();
				    infile.seekg(0, std::ios::beg);
				    existingContent.resize(fileSize);
				    infile.read(existingContent.data(), fileSize);
				    infile.close();
				}
				
				// Find the first null character in the existing content
				size_t existingSize = existingContent.size();
				size_t newSize = content.size();
				size_t startPos = existingSize; // Start appending from the end of the existing content

				// Find the position from where the new content starts
				for (size_t i = 0; i < std::min(existingSize, newSize); ++i)
				{
				    if (existingContent[i] != content[i])
				    {
				        startPos = i;
				        break;
				    }
				}

				
				std::ofstream outfile(filePath.c_str(), std::ios::binary | std::ios::app);
				if (outfile)
				{
				    outfile.write(content.data() + startPos, newSize - startPos);
				    outfile.close();
				    if (connection_state != CHUNKED)
				        _serveHtmlContent("<h1>File uploaded successfully</h1>", 200, "OK");
				}
				else
				{
				    std::cerr << "Error opening file for writing: " << fileName << std::endl;
				    _handleErrorPage(400, location);
				}
            }
        }
    }
}

void	RequestHandler::_handlePostRequest(const std::string& rootDir, const Location& location, ConnectionState& connection_state)
{
	std::istringstream iss(_request.getHeaders().at("Content-Length"));
	int	content_length;
	iss >> content_length;

	(void)rootDir;
	size_t max_size_bytes = _config.getClientMaxBodySize() * 1024 * 1024;
	// check the content length header exists
	if (_request.getHeaders().find("Content-Length") == _request.getHeaders().end())
	{
		try 
		{
			_respond_with_error(411, "Length Required", location);
		}
		catch (const std::exception& e)
		{
			_DefaultErrorPage(411);
		}
	}
	else if ((size_t)content_length > max_size_bytes)
	{
		try
		{
			_respond_with_error(413, "Forbidden", location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(413);
		}
		return ;
	}
	// check the content type header exists
	else if (_request.getHeaders().find("Content-Type") == _request.getHeaders().end())
	{
		try 
		{
			_respond_with_error(400, "Bad Request", location);
		}
		catch (const std::exception& e)
		{
			_DefaultErrorPage(400);
		}
	}
	if (_request.getHeaders().at("Content-Type").find("multipart/form-data") != std::string::npos)
	{
		std::string	boundary_delimiter =_ExtractBoundaryDelimiter();
		// std::string body = _getExactBody(_request.getBody().data(), content_length); 
		std::vector<char> body = _request.getBody();

		// delete the boundary delimiter from the body

		std::string bodyStr(body.begin(), body.end());

    	// Remove the boundary delimiter from the body
    	size_t pos = bodyStr.find(boundary_delimiter);
   		if (pos != std::string::npos)
		{
        	bodyStr.erase(pos, boundary_delimiter.length());
    	}

    	// Convert the string back to a vector of chars
		if (!bodyStr.empty())
		{
    		std::vector<char> newBody(bodyStr.begin(), bodyStr.end() - boundary_delimiter.length() - 6);
			_ParseMultipartFormData(newBody, boundary_delimiter, location, connection_state);
		}
	}
}
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 12:08:56 by moetienn          #+#    #+#             */
/*   Updated: 2025/01/20 12:21:11 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"

std::string	_generateDirectoryListing(const std::string& path, const std::string& uri)
{
	std::stringstream html;
	html << "<html><head><title>Directory Listing</title></head><body>";
	html << "<h1>Directory Listing for " << path << "</h1>";
	html << "<ul>";

	DIR* dir = opendir(path.c_str());
	if (dir)
	{
		struct dirent* entry;
		
		while ((entry = readdir(dir)) != NULL)
		{
			if (entry->d_name[0] != '.') // Skip hidden files
			{
				if (entry->d_type == DT_DIR)
				{
					std::string file_path = uri + entry->d_name;
					html << "<li><a href=\"" << file_path << "\">" << entry->d_name << "/</a></li>";
				}
				else
				{
					std::string file_path = uri + "/" + entry->d_name;
					const std::string prefix = "www";
					if (file_path.find(prefix) == 0)
					{
    					file_path = file_path.substr(prefix.length());
					}
					html << "<li><a href=\"" << file_path << "\">" << entry->d_name << "</a></li>";
				}
			}
		}
		closedir(dir);
	}

	html << "</ul></body></html>";
	return (html.str());
}

void	RequestHandler::_handleDirectoryListing(const std::string& path, const std::string& uri)
{
	std::string html_content = _generateDirectoryListing(path, uri);
	_serveHtmlContent(html_content, 200, "OK");
}


// Function to decode URL-encoded strings (e.g. %20 -> ' ')
std::string urlDecode(const std::string &value)
{
    std::ostringstream decoded;
    for (std::size_t i = 0; i < value.length(); ++i)
	{
        if (value[i] == '%' && i + 2 < value.length())
		{
            std::istringstream hex(value.substr(i + 1, 2));
            int hexValue;
            hex >> std::hex >> hexValue;
            decoded << static_cast<char>(hexValue);
            i += 2;
        }
		else if (value[i] == '+')
		{
            decoded << ' ';
        }
		else
		{
            decoded << value[i];
        }
    }
    return decoded.str();
}

void	RequestHandler::_handleFileOrDirectoryRequest(const std::string& full_path, const std::string& uri, const Location& location)
{
	std::string decoded_path = urlDecode(full_path);
	bool is_return = false;
	
	struct stat path_stat;

	if (!location.getReturn().empty())
	{
		std::string return_path = location.getReturn();
		std::string root_path = location.getRoot();
		std::string new_path = root_path + return_path;
		decoded_path = new_path;
		std::cout << "decoded_path: " << decoded_path << std::endl;
		is_return = true;
	}
	if (stat(decoded_path.c_str(), &path_stat) == 0)
	{
		if (S_ISDIR(path_stat.st_mode))
		{
			if (location.getAutoIndex() == 1)
			{
				_handleDirectoryListing(decoded_path, uri);
			}
			else
			{
				try
				{
					_respond_with_error(403, "Forbidden", location);
				}
				catch (std::exception& e)
				{
					_DefaultErrorPage(403);
				}
			}
		}
		else if (S_ISREG(path_stat.st_mode))
		{
			std::string extension = decoded_path.substr(decoded_path.find_last_of(".") + 1);
			if (access(decoded_path.c_str(), R_OK) == 0)
			{
				if (extension == "html" && is_return == false)
				{
					_respond_with_html(decoded_path.c_str(), 200, "OK");
				}
				else if (extension == "html" && is_return == true)
				{
					_respond_with_html(decoded_path.c_str(), 302, "OK");
				}
				else
        		{
					// Download file
            		std::ifstream file(decoded_path.c_str(), std::ios::binary);
            		if (file)
            		{
            		    // Get the file size
            		    file.seekg(0, std::ios::end);
            		    std::streamsize fileSize = file.tellg();
            		    file.seekg(0, std::ios::beg);
		
            		    std::vector<char> buffer(fileSize);
            		    if (file.read(buffer.data(), fileSize))
            		    {
            		        std::ostringstream responseStream;
            		        responseStream << "HTTP/1.1 200 OK\r\n";
            		        responseStream << "Content-Type: application/octet-stream\r\n";
            		        responseStream << "Content-Disposition: attachment; filename=\"" << full_path.substr(full_path.find_last_of("/") + 1) << "\"\r\n";
            		        responseStream << "Content-Length: " << fileSize << "\r\n";
            		        responseStream << "\r\n";
            		        responseStream.write(buffer.data(), fileSize);

							response = responseStream.str();
            		    }
            		    else
            		    {
            		        std::cerr << "Error reading file content: " << full_path << std::endl;
            		        _respond_with_error(500, "Internal Server Error", location);
            		    }
            		}
				}
			}
			else
			{
				try
				{
					_respond_with_error(403, "Forbidden", location);
				}
				catch (std::exception& e)
				{
					_DefaultErrorPage(403);
				}
			}
		}
		else
		{
			try
			{
				_respond_with_error(404, "Not Found", location);
			}
			catch (std::exception& e)
			{
				_DefaultErrorPage(404);
			}
			Logger::logMsg(ERROR, "No page FOUND %d - code", 404);
		}
	}
	else
	{
		try {
			_respond_with_error(404, "Not Found", location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(404);
		}
		Logger::logMsg(ERROR, "No page FOUND %d - code", 404);
	}
}

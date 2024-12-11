/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 12:08:56 by moetienn          #+#    #+#             */
/*   Updated: 2024/12/11 15:12:31 by marvin           ###   ########.fr       */
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
	std::cout << "PATH: " << path << std::endl;
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


void	RequestHandler::_handleRootDirectoryRequest(const std::string& rootDir, const std::string& uri, const Location& location)
{
	if (location.getIndex().empty() && location.getAutoIndex() == 1)
	{
		_handleDirectoryListing(rootDir, uri);
	}
	else if (location.getIndex().empty() && location.getAutoIndex() == 0)
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
	else
	{
		_respond_with_html(rootDir + location.getIndex(), 200, "OK");
	}
}

void	RequestHandler::_handleFileOrDirectoryRequest(const std::string& full_path, const std::string& uri, const Location& location)
{
	struct stat path_stat;
	if (stat(full_path.c_str(), &path_stat) == 0)
	{
		if (S_ISDIR(path_stat.st_mode))
		{
			if (location.getAutoIndex() == 1)
			{
				_handleDirectoryListing(full_path, uri);
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
			std::cout << "IS REG" << std::endl;
			if (access(full_path.c_str(), R_OK) == 0)
				_respond_with_html(full_path.c_str(), 200, "OK");
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

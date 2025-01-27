/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Delete.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 12:16:07 by moetienn          #+#    #+#             */
/*   Updated: 2025/01/28 05:28:53 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"

void RequestHandler::_handleDeleteRequest(const std::string& full_path, const Location& location)
{
    std::string decoded_path = full_path;
    size_t pos = 0;
    while ((pos = decoded_path.find("%20", pos)) != std::string::npos)
	{
        decoded_path.replace(pos, 3, " ");
        pos += 1;
    }
	
    std::string trimmed_uri = decoded_path;
    pos = trimmed_uri.find_last_of('/');
    trimmed_uri = trimmed_uri.substr(0, pos);
    
    if (access(trimmed_uri.c_str(), W_OK) != 0 || access(trimmed_uri.c_str(), X_OK) != 0)
	{
		try
		{
			_respond_with_error(403, "Forbidden", location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(403);
		}
        return;
    }
    
    if (access(decoded_path.c_str(), F_OK) != 0)
	{
		try
		{
			_respond_with_error(404, "Not Found", location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(404);
		}
        return;
    }

	int fd = open(decoded_path.c_str(), O_WRONLY | O_EXCL);
    if (fd == -1)
	{
		try
		{
        	_respond_with_error(423, "Locked", location);
			close(fd);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(423);
		}
    	close(fd);
        return;
    }
    close(fd);

    if (remove(decoded_path.c_str()) == 0)
	{
        _serveHtmlContent("<h1>File deleted successfully</h1>", 200, "OK");
    }
	else
	{
		try
		{
			_respond_with_error(500, "Internal Server Error", location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(500);
		}
    }
}
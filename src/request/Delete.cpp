/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   delete.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 12:16:07 by moetienn          #+#    #+#             */
/*   Updated: 2024/10/18 12:16:39 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"

void	RequestHandler::_handleDeleteRequest(int client_fd, const std::string& full_path, const Location& location)
{
	std::string trimmed_uri = full_path.c_str();
	std::cout << "FULL PATH: " << full_path << std::endl;
	size_t pos = trimmed_uri.find_last_of('/');
	trimmed_uri = trimmed_uri.substr(0, pos);
	std::cout << "TRIMMED URI in delete request: " << trimmed_uri << std::endl;
	if (access(trimmed_uri.c_str(), W_OK) != 0 || access(trimmed_uri.c_str(), X_OK) != 0)
	{
		std::cout << "Dont have permission to read on the directory" << std::endl;
		// if the file is not readable return 403
		try
		{
			_respond_with_error(client_fd, 403, "Forbidden", location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(client_fd, 403);
		}
	}
	else if (access(full_path.c_str(), F_OK) != 0)
	{
		try
		{
			_respond_with_error(client_fd, 404, "Not Found", location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(client_fd, 404);
		}
	}
	else if (remove(full_path.c_str()) == 0)
	{
		_serveHtmlContent(client_fd, "<h1>File deleted successfully</h1>", 200, "OK");
	}
	std::cout << "DELETE REQUEST" << std::endl;
}
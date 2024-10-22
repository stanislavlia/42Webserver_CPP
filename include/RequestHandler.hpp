/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/23 12:58:30 by moetienn          #+#    #+#             */
/*   Updated: 2024/10/22 09:15:08 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

#include "Request.hpp"
#include "ServerParam.hpp"
#include "ConfigParser.hpp"
#include "Server.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>

#include <vector>
#include <string>

class RequestHandler
{
	private:
		Request				_request;
		ServerParam&		_config;
		int					_socket;

		void		_ParseMultipartFormData(int client_fd, const std::string& body, const std::string& boundary_delimiter, const Location& location);
		std::string	_ExtractBoundaryDelimiter();
		void		_handlePostRequest(int client_fd, const std::string& rootDir, const Location& location);
		void		_handleDeleteRequest(int client_fd, const std::string& full_path, const Location& location);
		void		_handleFileOrDirectoryRequest(int client_fd, const std::string& full_path, const std::string& uri, const Location& location);
		std::string	_render_html(const std::string& path);
		void		_serveHtmlContent(int client_fd, const std::string& html_content, int status_code, const std::string& status_message);
		void		_respond_with_html(int socket, const std::string& path, int status_code, const std::string& status_message);
		void 		_respond_with_error(int socket, int status_code, const std::string& status_message, const Location& location);
		void		_handleInvalidRequest(int socket, int error, const Location& location);
		void		_handleRootDirectoryRequest(int client_fd, const std::string& rootDir, const std::string& uri, const Location& location);
		void		_handleDirectoryListing(int client_fd, const std::string& path, const std::string& uri);
		void		_DefaultErrorPage(int client_fd, int status_code);
		std::string	_getExactBody(const std::string& body, int content_length);
	public:
		RequestHandler(int socket, Request& request, ServerParam& config);
		RequestHandler(const RequestHandler& src);
		RequestHandler& operator=(const RequestHandler& rhs);
		~RequestHandler();


		void    handleRequest();
};

#endif
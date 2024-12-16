/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/23 12:58:30 by moetienn          #+#    #+#             */
/*   Updated: 2024/12/16 12:38:43 by marvin           ###   ########.fr       */
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
		std::string			response;

		void		_ParseMultipartFormData(const std::string& body, const std::string& boundary_delimiter, const Location& location);
		void		_handleCgiRequest(const std::string& full_path, const Location& location, const std::string& request_uri);
		std::string	_ExtractBoundaryDelimiter();
		void		_handlePostRequest(const std::string& rootDir, const Location& location);
		void		_handleDeleteRequest(const std::string& full_path, const Location& location);
		void		_handleFileOrDirectoryRequest(const std::string& full_path, const std::string& uri, const Location& location);
		std::string	_render_html(const std::string& path);
		void		_serveHtmlContent(const std::string& html_content, int status_code, const std::string& status_message);
		void		_respond_with_html(const std::string& path, int status_code, const std::string& status_message);
		void 		_respond_with_error(int status_code, const std::string& status_message, const Location& location);
		void		_handleInvalidRequest(int error, const Location& location);
		// void		_handleRootDirectoryRequest(const std::string& rootDir, const std::string& uri, const Location& location);
		void		_handleDirectoryListing(const std::string& path, const std::string& uri);
		void		_DefaultErrorPage(int status_code);
		std::string	_getExactBody(const std::string& body, int content_length);

		// CGI 

		void		_prepareAndExecuteCgiScript(const std::string& full_path, const std::vector<std::string>& args);
		int			_waitForChildProcess(pid_t pid);
		void		_parseRequestUri(const std::string& request_uri, std::string& full_path, std::string& query_string);
		void		_handleChildProcess(int pipefd[2], const std::string& request_uri);
		void		_handleParentProcess(int pipefd[2], pid_t pid, const std::string& request_uri, const Location& matched_location);
		bool		_createPipe(int pipefd[2]);
		void		_closePipe(int pipefd[2]);
		bool		_handleCgiError(const std::string& full_path, const Location& location, int exit_status);
		
		// ERROR 

		void		_handleErrorPage(int status_code, const Location& location);

	public:
		RequestHandler(int socket, Request& request, ServerParam& config);
		RequestHandler(const RequestHandler& src);
		RequestHandler& operator=(const RequestHandler& rhs);
		~RequestHandler();


		void    handleRequest();
		std::string getResponse() const;
};

#endif
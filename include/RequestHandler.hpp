/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/23 12:58:30 by moetienn          #+#    #+#             */
/*   Updated: 2025/01/05 14:44:46 by marvin           ###   ########.fr       */
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
#include "Monitor.hpp"

#include <vector>
#include <string>

class RequestHandler
{
	private:
		Request				_request;
		ServerParam&		_config;
		int					_socket;
		Monitor				*monitor;
		std::string			response;


		std::string	buildRequestPath(const Location& location, std::string& request_uri);
		bool		findMatchingLocation(const std::string& request_uri, Location& matched_location, size_t& matched_index);
		void		_ParseMultipartFormData(const std::string& body, const std::string& boundary_delimiter, const Location& location);
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

		void		_handleCgiRequest(const std::string& full_path, const Location& location, const std::string& request_uri, int client_fd, std::vector<int>& client_fds);
		void initializeCgiState(int client_fd);
    	void parseFullPath(const std::string& full_path, std::string& script_name, std::string& query_string);
    	void setContentLength(std::string& content_length);
		// void createPipe(int pipefd[2]);
		// pid_t forkAndExecuteCgiScript(int pipefd[2], const std::string& script_name);
		// void writePostDataToChild(int stdin_fd);
		// void handleCgiResponse(int pipefd[2], pid_t pid, const std::string& request_uri, const Location& location, int client_fd);
    	void createPipes(int pipefd[2], int stdin_pipe[2]);
    	pid_t forkAndExecuteCgiScript(int pipefd[2], int stdin_pipe[2], const std::string& script_name);
    	void writePostDataToChild(int stdin_pipe[2]);
    	void handleCgiResponse(int pipefd[2], pid_t pid, const std::string& request_uri, const Location& location, int client_fd, std::vector<int>& client_fds);
    	int _waitForChildProcess(pid_t pid);
    	// void _serveHtmlContent(const std::string& content, int status_code, const std::string& status_message);
    	void _handleCgiError(const std::string& request_uri, const Location& location, int status);
    	void setEnvironmentVariables(const std::string& query_string, const std::string& full_path,
                                 const std::string& content_length, const Location& location,
                                 const std::string& request_uri);

		// void		buildPostResponse(const std::string& response, const Location& location);
		// void		setEnvironmentVariables(const std::string& query_string, const std::string& script_name, const std::string& content_length, const Location& location, const std::string& request_uri);
		// void		_prepareAndExecuteCgiScript(const std::string& full_path, const std::vector<std::string>& args);
		// int			_waitForChildProcess(pid_t pid);
		// void		_parseRequestUri(const std::string& request_uri, std::string& full_path, std::string& query_string);
		// void		_handleChildProcess(int pipefd[2], const std::string& request_uri);
		// void		_handleParentProcess(int pipefd[2], pid_t pid, const std::string& request_uri, const Location& matched_location);
		// bool		_createPipe(int pipefd[2]);
		// void		_closePipe(int pipefd[2]);
		// bool		_handleCgiError(const std::string& full_path, const Location& location, int exit_status);
		
		// ERROR 


		void		_handleErrorPage(int status_code, const Location& location);

	public:
		RequestHandler(int socket, Request& request, ServerParam& config, Monitor* mon);
		RequestHandler(const RequestHandler& src);
		RequestHandler& operator=(const RequestHandler& rhs);
		~RequestHandler();


		void    handleRequest(std::vector<int>& client_fds);
		std::string getResponse() const;
		// getter for cgi state
		// CgiState	getCgiState(int client_fd);
};

#endif
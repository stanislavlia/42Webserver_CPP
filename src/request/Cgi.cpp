#include "RequestHandler.hpp"
#include <sys/wait.h>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <iostream>

// Signal handler for CGI script timeout
void alarm_handler(int signum, siginfo_t *info, void *context)
{
	(void)signum;
	(void)context;
	pid_t pid = info->si_pid;
	if (pid > 0)
	{
		kill(pid, SIGKILL);
		Logger::logMsg(ERROR, "CGI script timed out");
	}
}

// Function to set the environment variables required by the CGI script
void    RequestHandler::setEnvironmentVariables(const std::string& query_string, const std::string& script_name, const std::string& content_length, const Location& location, const std::string& request_uri)
{
	(void)script_name;
	(void)location;
	(void)request_uri;
	setenv("REQUEST_METHOD", _request.getMethod().c_str(), 1);
	std::cout << "REQUEST_METHOD: " << _request.getMethod() << std::endl;
	if (!query_string.empty())
	{
		setenv("QUERY_STRING", query_string.c_str(), 1);
	}
	// setenv("QUERY_STRING", query_string.c_str(), 1);
	setenv("CONTENT_LENGTH", content_length.c_str(), 1);
	std::cout << "CONTENT LENGTH: " << content_length << std::endl;
	setenv("HTTP_HOST", "localhost", 1);
	setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
	// Set other necessary environment variables similarly
}

bool RequestHandler::_createPipe(int pipefd[2])
{
	if (pipe(pipefd) == -1)
	{
		Logger::logMsg(ERROR, "Pipe error");
		return false;
	}
	return true;
}

int RequestHandler::_waitForChildProcess(pid_t pid)
{
	int status = 0;
	waitpid(pid, &status, 0);

	if (WIFEXITED(status))
	{
		int exit_status = WEXITSTATUS(status);
		if (exit_status != 0)
		{
			Logger::logMsg(ERROR, "CGI script exited with status %d", exit_status);
		}
		return exit_status;
	}
	else
	{
		Logger::logMsg(ERROR, "CGI script did not exit normally");
	}
	return status;
}

bool RequestHandler::_handleCgiError(const std::string& request_uri, const Location& location, int exit_status)
{
	std::string file_path;

	size_t pos = request_uri.find("?");
	if (pos != std::string::npos)
	{
		file_path = request_uri.substr(0, pos);
	}
	else
	{
		file_path = location.getRoot();
	}
	if (access(file_path.c_str(), F_OK) != 0)
	{
		try
		{
			_respond_with_error(404, "Not Found", location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(404);
		}
	}
	else if (access(file_path.c_str(), X_OK) != 0 || access(file_path.c_str(), R_OK) != 0)
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
	else if (exit_status == 120)
	{
		try
		{
			_respond_with_error(504, "Gateway Timeout", location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(504);
		}
	}
	else
	{
		try
		{
			_respond_with_error(400, "Bad Request", location);
		}
		catch (std::exception& e)
		{
			_DefaultErrorPage(400);
		}
	}
	return false;
}

void RequestHandler::_handleCgiRequest(const std::string& full_path, const Location& location, const std::string& request_uri)
{
	std::string query_string;
    size_t pos = full_path.find("?");
	std::string script_name;
	std::cout << "full_path: " << full_path << std::endl;
	std::cout << "request_uri: " << request_uri << std::endl;
	std::cout << "location.getRoot(): " << location.getRoot() << std::endl;
    if (pos != std::string::npos)
    {
		script_name = full_path.substr(0, pos);
        query_string = full_path.substr(pos + 1);
    }
	else
	{
		script_name = full_path;
		query_string = "";
	}

	std::string content_length;
	if (_request.getHeaders().find("Transfer-Encoding") != _request.getHeaders().end())
	{
		std::ostringstream oss;
		oss << _request.getBody().length();
		content_length = oss.str();
	}
	else
	{
		content_length = _request.getHeaders().at("Content-Length");
		// content_length = "0";
	}
    // std::string content_length = _request.getHeaders().at("Content-Length");
	setEnvironmentVariables(query_string, full_path, content_length, location, request_uri);
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        Logger::logMsg(ERROR, "Pipe error");
        return;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        Logger::logMsg(ERROR, "Fork error");
        return;
    }

    if (pid == 0)
    {
        // Child process
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        int stdin_pipe[2];
        if (pipe(stdin_pipe) == -1)
        {
            Logger::logMsg(ERROR, "Pipe error");
            return;
        }

        	pid_t stdin_pid = fork();
        	if (stdin_pid == -1)
        	{
        	    Logger::logMsg(ERROR, "Fork error");
        	    return;
        	}

        	if (stdin_pid == 0)
        	{
        	    // Child process to write POST data to stdin
        	    close(stdin_pipe[0]);
        	    write(stdin_pipe[1], _request.getBody().c_str(), _request.getBody().length());
        	    close(stdin_pipe[1]);
        	    exit(0);
		}
        else
        {
            // Parent process to execute CGI script
            close(stdin_pipe[1]);
            dup2(stdin_pipe[0], STDIN_FILENO);
            close(stdin_pipe[0]);

            std::string interpreter = "/usr/bin/python3";
            std::string script = script_name;
            char *exec_args[] = {
                const_cast<char*>(interpreter.c_str()),
                const_cast<char*>(script.c_str()),
                NULL
            };

            execv(interpreter.c_str(), exec_args);
            Logger::logMsg(ERROR, ("Execv failed: " + std::string(strerror(errno))).c_str());
            exit(1);
        }
    }
    else
    {
        // Parent process
        close(pipefd[1]);

        std::string response;
        char buffer[4096];
        int bytes_read;

		struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = alarm_handler;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGALRM, &sa, NULL);

        alarm(5);

        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0)
        {
            response.append(buffer, bytes_read);
        }

        close(pipefd[0]);

        int status = _waitForChildProcess(pid);
		std::cout << "status: " << status << std::endl;
		std::cout << "response: " << response << std::endl;
        if (status == 0)
        {
            _serveHtmlContent(response, 200, "OK");
        }
        else
        {
            _handleCgiError(request_uri, location, status);
        }
    }
}

void RequestHandler::buildPostResponse(const std::string& response, const Location& location)
{
	(void)location;

	std::map<std::string, std::string> headers = _request.getHeaders();
	std::istringstream iss(response);
	std::string line;
	std::map<std::string, std::string> newHeaders;

	// Add new Header from Script
	while (std::getline(iss, line))
	{
		std::istringstream headerStream(line);
		std::string key, value;
		if (std::getline(headerStream, key, ':') && std::getline(headerStream, value))
		{
			// Remove leading spaces from value
			value.erase(0, value.find_first_not_of(" \t"));
			newHeaders[key] = value;
		}
	}
	
	// Update headers in _request
	for (std::map<std::string, std::string>::iterator it = newHeaders.begin(); it != newHeaders.end(); ++it)
	{
		_request.addHeader(it->first, it->second);
	}

	if (newHeaders.find("Content-Disposition") == newHeaders.end())
	{
		// Add a default Content-Disposition header
		std::string filename = "\"default_filename.txt\""; // Set your default filename here
		_request.addHeader("Content-Disposition", "form-data; name=\"file\"; filename=" + filename);
	}

	headers = _request.getHeaders(); // Refresh the headers map

	if (headers.find("Content-Disposition") != headers.end())
	{
		std::string contentDisposition = headers["Content-Disposition"];
		size_t fileNamePos = contentDisposition.find("filename=\"");
		if (fileNamePos != std::string::npos)
		{
			size_t fileNameEnd = contentDisposition.find("\"", fileNamePos + 10);
			std::string fileName = contentDisposition.substr(fileNamePos + 10, fileNameEnd - fileNamePos - 10);
			std::string filePath = location.getRoot() + "/" + fileName;
			std::ofstream outfile(filePath.c_str());
			if (outfile)
			{
				outfile.write(_request.getBody().c_str(), _request.getBody().size());
				outfile.close();
				_serveHtmlContent(response, 200, "OK");
				// _serveHtmlContent2("<h1>File uploaded successfully</h1>", 200, "OK");
			}
			else
			{
				Logger::logMsg(ERROR, "Failed to open file for writing");
				_serveHtmlContent("<h1>Failed to upload file</h1>", 500, "Internal Server Error");
			}
		}
	}
}

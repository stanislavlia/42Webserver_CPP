#include "RequestHandler.hpp"
#include <sys/wait.h>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <iostream>

int Timeout = 0;

// Signal handler for CGI script timeout
void alarm_handler(int signum, siginfo_t *info, void *context)
{
	(void)signum;
	(void)context;
	pid_t pid = info->si_pid;
    if (pid > 0)
    {
        kill(pid, SIGKILL);
    }
    Timeout = 1;
}

// Function to set the environment variables required by the CGI script
void    RequestHandler::setEnvironmentVariables(const std::string& query_string, const std::string& script_name, const std::string& content_length, const Location& location, const std::string& request_uri)
{
	(void)script_name;
	(void)location;
	(void)request_uri;
	setenv("REQUEST_METHOD", _request.getMethod().c_str(), 1);
	if (!query_string.empty())
	{
		setenv("QUERY_STRING", query_string.c_str(), 1);
	}
	setenv("CONTENT_LENGTH", content_length.c_str(), 1);
	setenv("HTTP_HOST", "localhost", 1);
	setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
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

void	RequestHandler::_handleCgiError(const std::string& request_uri, const Location& location, int exit_status)
{
	std::string file_path;

	size_t pos = request_uri.find("?");
	if (pos != std::string::npos)
	{
		file_path = request_uri.substr(0, pos);
	}
	else
	{
		file_path = request_uri;
	}
	if (exit_status == 120)
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
	else if (access(file_path.c_str(), F_OK) != 0)
	{
        std::cout << "File not found" << std::endl;
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
}

void RequestHandler::parseFullPath(const std::string& full_path, std::string& script_name, std::string& query_string)
{
    size_t pos = full_path.find("?");
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
}

void RequestHandler::setContentLength(std::string& content_length)
{
    if (_request.getHeaders().find("Transfer-Encoding") != _request.getHeaders().end())
    {
        std::ostringstream oss;
        oss << _request.getBody().length();
        content_length = oss.str();
    }
    else
    {
        content_length = _request.getHeaders().at("Content-Length");
    }
}

pid_t RequestHandler::forkAndExecuteCgiScript(int cgi_in[2], int cgi_out[2], const std::string& script_name)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        Logger::logMsg(ERROR, "Fork error");
        return -1;
    }

    if (pid == 0)
    {
        // Child process
        close(cgi_out[0]);
        dup2(cgi_out[1], STDOUT_FILENO);
        close(cgi_out[1]);

        close(cgi_in[1]);
        dup2(cgi_in[0], STDIN_FILENO);
        close(cgi_in[0]);

        std::string interpreter = "/usr/bin/python3";
        char *exec_args[] = {
            const_cast<char*>(interpreter.c_str()),
            const_cast<char*>(script_name.c_str()),
            NULL
        };

        execv(interpreter.c_str(), exec_args);
        Logger::logMsg(ERROR, ("Execv failed: " + std::string(strerror(errno))).c_str());
        exit(1);
    }
    return pid;
}

void RequestHandler::writePostDataToChild(int cgi_in[2])
{
    if (_request.getMethod() == "POST")
    {
        // Parent process
        close(cgi_in[0]);

        // Write the POST data to the child's stdin
        write(cgi_in[1], _request.getBody().c_str(), _request.getBody().length());
        close(cgi_in[1]);
        monitor->incrementWriteCount();
    }
}

void RequestHandler::handleCgiResponse(int cgi_out[2], pid_t pid, const std::string& request_uri, const Location& location, int client_fd, std::vector<int>& client_fds)
{
    (void)client_fds;

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = alarm_handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        Logger::logMsg(ERROR, "Failed to set up SIGALRM handler");
        exit(1);
    }
    unsigned int timeout = 20;

    alarm(timeout);
    int status = _waitForChildProcess(pid);
    if (Timeout != 0)
    {
        status = 120;
        monitor->addCgiStatus(client_fd, status);
        _handleCgiError(request_uri, location, status);
        monitor->setCgiState(client_fd, NO_STATE);
        close(cgi_out[0]);
        kill (pid, SIGKILL);
        Timeout = 0;
    }
    else if (status != 0)
    {
        monitor->addCgiStatus(client_fd, status);
        _handleCgiError(request_uri, location, status);
        monitor->setCgiState(client_fd, NO_STATE);
        // monitor->removeCgiPipe(client_fd);
        close(cgi_out[0]);
    }
    else
    {
        monitor->addCgiStatus(client_fd, status);
        monitor->addCgiPipe(client_fd, cgi_out[0]);
    }
	if (monitor->getReadCount() > 0 && status == 0)
	{
		return ;
	}
}

void RequestHandler::_handleCgiRequest(const std::string& full_path, const Location& location, const std::string& request_uri, int client_fd, std::vector<int>& client_fds)
{
    std::string script_name;
    std::string query_string;
    parseFullPath(full_path, script_name, query_string);
    std::string content_length;
    setContentLength(content_length);
    setEnvironmentVariables(query_string, full_path, content_length, location, request_uri);

    int cgi_in[2];
    int cgi_out[2];
    createPipes(cgi_in, cgi_out);
    monitor->setCgiState(client_fd, CGI_INIT);

    pid_t pid = forkAndExecuteCgiScript(cgi_in, cgi_out, script_name);
    monitor->setCgiState(client_fd, CGI_WRITING);

    writePostDataToChild(cgi_in);
    monitor->setCgiState(client_fd, CGI_READING);
    if (monitor->getCgiState(client_fd) == CGI_READING)
    {
        handleCgiResponse(cgi_out, pid, request_uri, location, client_fd, client_fds);
    }
}


void RequestHandler::createPipes(int pipefd[2], int stdin_pipe[2])
{
    if (pipe(pipefd) == -1 || pipe(stdin_pipe) == -1)
    {
        Logger::logMsg(ERROR, "Pipe error");
        exit(1);
    }

}

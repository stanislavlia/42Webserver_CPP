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
        oss << _request.getBody().size();
        content_length = oss.str();
    }
    else
    {
        content_length = _request.getHeaders().at("Content-Length");
    }
}

pid_t RequestHandler::forkAndExecuteCgiScript(int cgi_in[2], int cgi_out[2], const std::string& script_name, const std::vector<std::string>& env_vars, const std::string& interpreter)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        Logger::logMsg(ERROR, "Fork error");
        return -1;
    }

    if (pid == 0)
    {
        close(cgi_out[0]);
        dup2(cgi_out[1], STDOUT_FILENO);
        close(cgi_out[1]);

        close(cgi_in[1]);
        dup2(cgi_in[0], STDIN_FILENO);
        close(cgi_in[0]);

        std::vector<char*> env_vars_cstr;
        for (size_t i = 0; i < env_vars.size(); i++)
        {
            env_vars_cstr.push_back(const_cast<char*>(env_vars[i].c_str()));
        }
        env_vars_cstr.push_back(NULL);

        char *exec_args[] = {
            const_cast<char*>(interpreter.c_str()),
            const_cast<char*>(script_name.c_str()),
            NULL
        };

        execve(interpreter.c_str(), exec_args, env_vars_cstr.data());
        Logger::logMsg(ERROR, ("Execv failed: " + std::string(strerror(errno))).c_str());
        exit(1);
    }
    return pid;
}

void RequestHandler::writePostDataToChild(int cgi_in[2])
{
    if (_request.getMethod() == "POST")
    {
        close(cgi_in[0]);

        write(cgi_in[1], _request.getBody().data(), _request.getBody().size());
        close(cgi_in[1]);
    }
}

void RequestHandler::handleCgiResponse(int cgi_out[2], pid_t pid, const std::string& request_uri, const Location& location, int client_fd, std::vector<int>& client_fds)
{
    (void)client_fds;
    Timeout = 0;
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = alarm_handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        Logger::logMsg(ERROR, "Failed to set up SIGALRM handler");
        exit(1);
    }
    unsigned int timeout = 30;

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
        close(cgi_out[0]);
    }
    else
    {
        monitor->addCgiStatus(client_fd, status);
        monitor->addCgiPipe(client_fd, cgi_out[0]);
        monitor->setCgiState(client_fd, CGI_READING);
    }
	if (monitor->getReadCount(client_fd) > 0 && status == 0)
	{
		return ;
	}
}

bool    RequestHandler::firstErrorCheck(const std::string& full_path, const Location& location)
{
    std::string file_path;

    size_t pos = full_path.find("?");
    if (pos != std::string::npos)
    {
        file_path = full_path.substr(0, pos);
    }
    else
    {
        file_path = full_path;
    }
    if (location.getAllowedExtensions().size() > 0)
    {
        bool allowed = false;
        for (size_t i = 0; i < location.getAllowedExtensions().size(); i++)
        {
            if (full_path.find(location.getAllowedExtensions()[i]) != std::string::npos)
            {
                allowed = true;
                break;
            }
        }
        if (!allowed)
        {
            try
            {
                _respond_with_error(403, "Forbidden", location);
            }
            catch (std::exception& e)
            {
                _DefaultErrorPage(403);
            }
            return true;
        }
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
        return true;
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
        return true;
    }
    return false;
}

void RequestHandler::_handleCgiRequest(const std::string& full_path, const Location& location, const std::string& request_uri, int client_fd, std::vector<int>& client_fds)
{
    std::string                 script_name;
    std::string                 content_length;
    std::vector<std::string>    env_vars;
    std::string                 query_string;
    std::string file_path = full_path.substr(0, full_path.find("?"));
    std::string extension = file_path.substr(file_path.find_last_of("."));
    std::string interpreter;

    if (firstErrorCheck(full_path, location) == true)
    {
        return ;
    }

    try
    {
        interpreter = location.getCgiInterpreter().at(extension);
    }
    catch (const std::out_of_range& e)
    {
        try
        {
            _respond_with_error(500, "Internal Server Error", location);
        }
        catch (std::exception& e)
        {
            _DefaultErrorPage(500);
        }
        return ;
    }

    if (_request.getMethod() == "POST")
    {
        monitor->incrementWriteCount(client_fd);
    }

    setContentLength(content_length);
    parseFullPath(full_path, script_name, query_string);
    env_vars.push_back("REQUEST_METHOD=" + _request.getMethod());
    if (!query_string.empty())
    {
        env_vars.push_back("QUERY_STRING=" + query_string);
    }
    env_vars.push_back("CONTENT_LENGTH=" + content_length);
    env_vars.push_back("HTTP_HOST=localhost");
    env_vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env_vars.push_back("USER=" + std::string(getenv("USER")));

    int cgi_in[2];
    int cgi_out[2];
    createPipes(cgi_in, cgi_out);
    monitor->setCgiState(client_fd, CGI_INIT);

    pid_t pid = forkAndExecuteCgiScript(cgi_in, cgi_out, script_name, env_vars, interpreter);
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

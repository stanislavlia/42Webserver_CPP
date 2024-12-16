#include "RequestHandler.hpp"
#include <sys/wait.h>

void    alarm_handler(int signum, siginfo_t *info, void *context)
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

std::vector<std::string>	parseQueryString(const std::string& query)
{
	std::vector<std::string> args;
	std::istringstream stream(query);
	std::string token;

	while (std::getline(stream, token, '&'))
	{
		std::string::size_type pos = token.find('=');
		if (pos != std::string::npos)
		{
			args.push_back(token.substr(pos + 1));
		}
	}
	return args;
}

void	RequestHandler::_handleCgiRequest(const std::string& full_path, const Location& location, const std::string& request_uri)
{
    if (full_path == location.getRoot())
    {
        _handleFileOrDirectoryRequest(full_path, request_uri, location);
        return;
    }

    int pipefd[2];
    if (!_createPipe(pipefd))
        return;

    pid_t pid = fork();
    if (pid == -1)
    {
        Logger::logMsg(ERROR, "Fork error");
        return;
    }

    if (pid == 0)
        _handleChildProcess(pipefd, request_uri);
    else
        _handleParentProcess(pipefd, pid, request_uri, location);
}

bool	RequestHandler::_createPipe(int pipefd[2])
{
    std::cout << "========= PIPE =============" << std::endl;
    if (pipe(pipefd) == -1)
    {
        Logger::logMsg(ERROR, "Pipe error");
        return false;
    }
    return true;
}

void	RequestHandler::_handleChildProcess(int pipefd[2], const std::string& request_uri)
{
    std::cout << "========= CHILD =============" << std::endl;
    close(pipefd[0]);

    std::string copy_full_path, query_string;
    _parseRequestUri(request_uri, copy_full_path, query_string);

    dup2(pipefd[1], STDOUT_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[1]);

    std::vector<std::string> args = parseQueryString(query_string);
    _prepareAndExecuteCgiScript(copy_full_path, args);
}

void	RequestHandler::_handleParentProcess(int pipefd[2], pid_t pid, const std::string& request_uri, const Location& matched_location)
{
	(void)request_uri;
    // std::cout << "========= PARENT =============" << std::endl;
    close(pipefd[1]);

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    alarm(5);

    std::cout << "========= PARENT =============" << std::endl;
    char buffer[BUFF_SIZE];
    int bytes_read;

    std::cout << "========= READ =============" << std::endl;
    while ((bytes_read = read(pipefd[0], buffer, BUFF_SIZE)) > 0)
        response.append(buffer, bytes_read);

    std::cout << "========= CLOSE =============" << std::endl;
    close(pipefd[0]);

    int exit_status = _waitForChildProcess(pid);
    std::cout << "exit status: " << exit_status << std::endl;
    std::cout << "========= RESPONSE =============" << std::endl;
	// _waitForChildProcess(pid);
    // std::cout << "========= RESPONSE =============" << std::endl;

    alarm(0);

	if (exit_status == 0)
    {
		_serveHtmlContent(response, 200, "OK");
    }
	else
    {
		_handleCgiError(request_uri, matched_location, exit_status);
    }
}

void	RequestHandler::_parseRequestUri(const std::string& request_uri, std::string& full_path, std::string& query_string)
{
    size_t pos = request_uri.find("?");
    if (pos != std::string::npos)
    {
        full_path = request_uri.substr(0, pos);
        query_string = request_uri.substr(pos + 1);
    }
    else
    {
        full_path = request_uri;
        query_string = "";
    }
}

void	RequestHandler::_prepareAndExecuteCgiScript(const std::string& full_path, const std::vector<std::string>& args)
{
    std::vector<char*> exec_args;
    exec_args.push_back(const_cast<char*>("/usr/bin/python3"));
    exec_args.push_back(const_cast<char*>(full_path.c_str()));
    for (size_t i = 0; i < args.size(); ++i)
        exec_args.push_back(const_cast<char*>(args[i].c_str()));
    exec_args.push_back(NULL);

	execv("/usr/bin/python3", &exec_args[0]);

	Logger::logMsg(ERROR, ("Execv failed: " + std::string(strerror(errno))).c_str());
    exit(1);
}

int	RequestHandler::_waitForChildProcess(pid_t pid)
{
    int status = 0;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
    {
        int exit_status = WEXITSTATUS(status);
        if (exit_status != 0)
            Logger::logMsg(ERROR, "CGI script exited with status %d", exit_status);
        return exit_status;
    }
    // else if (WIFSIGNALED(status))
    // {

    // }
    else
    {
        Logger::logMsg(ERROR, "CGI script did not exit normally");
    }
	return status;
}

bool	RequestHandler::_handleCgiError(const std::string& request_uri, const Location& location, int exit_status)
{
	std::cout << "========= CGI ERROR =============" << std::endl;
	std::string file_path;

	size_t pos = request_uri.find("?");
	if (pos != std::string::npos)
	{
		file_path = request_uri.substr(0, pos);
		// std::cout << "File path: " << file_path << std::endl;
	}
	else
	{
		file_path = request_uri;
	}
	if (access(file_path.c_str(), F_OK) != 0)
	{
		std::cout << "File does not exist" << std::endl;
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
		std::cout << "File is not executable" << std::endl;
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
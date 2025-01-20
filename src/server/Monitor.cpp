#include "Monitor.hpp"
#include <algorithm>
#include <iostream>

Monitor::Monitor() {
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&copy_read_fds);
    FD_ZERO(&copy_write_fds);

}

void Monitor::addReadFd(int fd) {
    FD_SET(fd, &read_fds);
}

void Monitor::addWriteFd(int fd) {
    FD_SET(fd, &write_fds);
}

void Monitor::removeReadFd(int fd) {
    FD_CLR(fd, &read_fds);
}

void Monitor::removeWriteFd(int fd) {
    FD_CLR(fd, &write_fds);
}

fd_set& Monitor::getReadFds() {
    return read_fds;
}

fd_set& Monitor::getWriteFds() {
    return write_fds;
}

fd_set& Monitor::getCopyReadFds() {
    return copy_read_fds;
}

fd_set& Monitor::getCopyWriteFds() {
    return copy_write_fds;
}

void    Monitor::incrementReadCount(int client_fd) {
    std::map<int, int>::iterator it = read_counts.find(client_fd);
    if (it != read_counts.end()) {
        it->second++;
    } else {
        read_counts[client_fd] = 1;
    }
}

void    Monitor::incrementWriteCount(int client_fd) {
    std::map<int, int>::iterator it = write_counts.find(client_fd);
    if (it != write_counts.end()) {
        it->second++;
    } else {
        write_counts[client_fd] = 1;
    }
}

int Monitor::getReadCount(int client_fd) const {
    std::map<int, int>::const_iterator it = read_counts.find(client_fd);
    if (it != read_counts.end()) {
        return it->second;
    }
    return 0;
}

int Monitor::getWriteCount(int client_fd) const {
    std::map<int, int>::const_iterator it = write_counts.find(client_fd);
    if (it != write_counts.end()) {
        return it->second;
    }
    return 0;
}

void Monitor::resetCounts() {
    read_counts.clear();
    write_counts.clear();
}

void Monitor::addCgiPipe(int client_fd, int pipe_fd) {
    cgi_pipes[client_fd] = pipe_fd;
}

int Monitor::getCgiPipe(int client_fd) const {
    std::map<int, int>::const_iterator it = cgi_pipes.find(client_fd);
    if (it != cgi_pipes.end()) {
        return it->second;
    }
    return -1;
}

void Monitor::removeCgiPipe(int client_fd) {
    cgi_pipes.erase(client_fd);
}

void Monitor::setCgiState(int client_fd, CgiState state) {
    _cgi_states[client_fd] = state;
}

CgiState Monitor::getCgiState(int client_fd) const {
    std::map<int, CgiState>::const_iterator it = _cgi_states.find(client_fd);
    if (it != _cgi_states.end()) {
        return it->second;
    }
    return NO_STATE;
}

void Monitor::removeCgiState(int client_fd)
{
    _cgi_states.erase(client_fd);
}

void Monitor::addCgiStatus(int client_fd, int status)
{
    cgi_status[client_fd] = status;
}

int Monitor::getCgiStatus(int client_fd) const
{
    std::map<int, int>::const_iterator it = cgi_status.find(client_fd);
    if (it != cgi_status.end())
    {
        return it->second;
    }
    return -1;
}

void Monitor::removeCgiStatus(int client_fd)
{
    cgi_status.erase(client_fd);
}
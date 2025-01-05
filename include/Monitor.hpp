#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <sys/types.h>
#include <sys/select.h>
#include <map>

enum CgiState
{
    NO_STATE,
    CGI_INIT,
    CGI_READING,
    CGI_WRITING,
    CGI_ERROR
};

class Monitor
{
    public:
        Monitor();

        // Methods to manage fd_set
        void addReadFd(int fd);
        void addWriteFd(int fd);
        void removeReadFd(int fd);
        void removeWriteFd(int fd);

        // Methods to get fd_set
        fd_set& getReadFds();
        fd_set& getWriteFds();
        fd_set& getCopyReadFds();
        fd_set& getCopyWriteFds();

        // Methods to manage read/write counts
        void incrementReadCount();
        void incrementWriteCount();
        int getReadCount() const;
        int getWriteCount() const;
        void resetCounts();

        // Methods to manage CGI pipes
        void addCgiPipe(int client_fd, int pipe_fd);
        int getCgiPipe(int client_fd) const;
        void removeCgiPipe(int client_fd);

        // Methods to manage CGI states
        void setCgiState(int client_fd, CgiState state);
        CgiState getCgiState(int client_fd) const;
        void removeCgiState(int client_fd);

        // Methods to manage CGI PIDs
        void addCgiStatus(int client_fd, int status);
        int getCgiStatus(int client_fd) const;
        void removeCgiStatus(int client_fd);

    private:
        fd_set read_fds;
        fd_set write_fds;
        fd_set copy_read_fds;
        fd_set copy_write_fds;
        std::map<int, int> cgi_pipes;
        std::map<int, int> cgi_status;
        int reading_count;
        int writing_count;
        std::map<int, CgiState> _cgi_states;
};

#endif // MONITOR_HPP
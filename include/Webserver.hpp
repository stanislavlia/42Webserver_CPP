#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <string>           // For using std::string
#include <vector>           // For using std::vector
#include <map>              // For using std::map
#include <set>              // For using std::set
#include <iostream>         // For standard input/output streams
#include <sstream>          // For string stream operations
#include <cstdlib>
#include <fcntl.h>
#include <stdlib.h>
#include <cstring>
#include <fstream>

/* System Headers */
#include <sys/socket.h>     // For socket functions and types
#include <sys/types.h>      // For data types used in system calls
#include <sys/select.h>     // For select() system call and fd_set
#include <sys/time.h>       // For timeval structure (used in select)
#include <netinet/in.h>     // For sockaddr_in and other network types (IPv4)
#include <arpa/inet.h>      // For inet_pton and inet_ntop functions
#include <unistd.h>         // For close() and other system calls
#include <fcntl.h>          // For manipulating file descriptors (e.g., fcntl)
#include <signal.h>         // For handling signals
#include <errno.h>          // For handling error codes


#define PORT 8012
#define BUFF_SIZE 1024
#define CONN_QUEUE 4




#endif

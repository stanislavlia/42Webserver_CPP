#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <map>
#include <string>
#include <cstdarg>
#include <iostream>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <string>

#define LIGHT_RED      "\x1B[91m"
#define WHITE          "\x1B[37m"
#define DARK_GREY      "\x1B[90m"
#define YELLOW         "\x1B[33m"



enum LogPriority{
    DEBUG,
    INFO,
    WARNING,
    ERROR,  
};



class Logger {
public:
    Logger();
    ~Logger();

    static std::map<LogPriority, std::string> prio_str;
    static std::map<LogPriority, const char*> prior_to_color;

    static void logMsg(LogPriority prio,  const char* msg, ...);

private:
    static std::string    getTimeStamp();
    static std::map<LogPriority, std::string> initPriotToStrMap();
    static std::map<LogPriority, const char*> initPriotToColorMap();
};


#endif
#include "Logger.hpp"

// Initialize the priority string map
std::map<LogPriority, std::string> Logger::prio_str = Logger::initPriotToStrMap();
std::map<LogPriority,  const char*>  Logger::prior_to_color = Logger::initPriotToColorMap();

Logger::Logger() {}

Logger::~Logger() {}

std::map<LogPriority, std::string> Logger::initPriotToStrMap() 
{
    std::map<LogPriority, std::string> p_map;
    p_map[DEBUG] = "[DEBUG]   ";
    p_map[INFO] = "[INFO]    ";
    p_map[WARNING] = "[WARNING] ";
    p_map[ERROR] = "[ERROR]   ";
    return p_map;
}

std::map<LogPriority, const char*> Logger::initPriotToColorMap()
{
    std::map<LogPriority, const char*> p_to_color_map;

    p_to_color_map[DEBUG] = DARK_GREY;
    p_to_color_map[INFO] = WHITE;
    p_to_color_map[WARNING] = YELLOW;
    p_to_color_map[ERROR] = LIGHT_RED;

    return p_to_color_map;
}

void Logger::logMsg(LogPriority prio,  const char* msg, ...) 
{
    va_list args;
    va_start(args, msg);
    const char* color;

    color = prior_to_color[prio];
    std::cout << color << prio_str[prio];

    vprintf(msg, args);

    std::cout << WHITE << std::endl;
    va_end(args);
}


#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <sstream>
#include <mutex>

enum class LogLevel {
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3
};

class Logger {
public:
    static Logger& getInstance();

    void setLevel(LogLevel level);
    void log(LogLevel level, const std::string& file, int line, const std::string& msg);

private:
    Logger() : level_(LogLevel::DEBUG) {}
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string levelStr(LogLevel level) const;
    std::string timestamp() const;

    LogLevel level_;
    std::mutex mutex_;
};

#define LOG_DEBUG(msg) do { \
    std::ostringstream _oss; _oss << msg; \
    Logger::getInstance().log(LogLevel::DEBUG, __FILE__, __LINE__, _oss.str()); \
} while(0)

#define LOG_INFO(msg) do { \
    std::ostringstream _oss; _oss << msg; \
    Logger::getInstance().log(LogLevel::INFO, __FILE__, __LINE__, _oss.str()); \
} while(0)

#define LOG_WARN(msg) do { \
    std::ostringstream _oss; _oss << msg; \
    Logger::getInstance().log(LogLevel::WARN, __FILE__, __LINE__, _oss.str()); \
} while(0)

#define LOG_ERROR(msg) do { \
    std::ostringstream _oss; _oss << msg; \
    Logger::getInstance().log(LogLevel::ERROR, __FILE__, __LINE__, _oss.str()); \
} while(0)

#endif // LOGGER_H

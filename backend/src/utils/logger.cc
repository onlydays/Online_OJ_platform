#include "logger.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

std::string Logger::levelStr(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}

std::string Logger::timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

void Logger::log(LogLevel level, const std::string& file, int line,
                 const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (level < level_) return;

    std::ostream& out = (level >= LogLevel::WARN) ? std::cerr : std::cout;

    const char* filename = file.c_str();
    const char* slash = filename;
    for (const char* p = filename; *p; ++p) {
        if (*p == '/' || *p == '\\') slash = p + 1;
    }

    out << "[" << timestamp() << "] "
        << "[" << levelStr(level) << "] "
        << "[" << slash << ":" << line << "] "
        << msg << std::endl;
}

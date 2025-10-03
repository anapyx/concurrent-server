#include "logger.h"
#include <iostream>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    if (logFile_.is_open()) {
        logFile_.close();
    }
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open()) {
        logFile_.close();
    }
    logFile_.open(filename, std::ios::app);
}

void Logger::setLevel(Level level) {
    std::lock_guard<std::mutex> lock(mutex_);
    currentLevel_ = level;
}

void Logger::debug(const std::string& message) {
    log(Level::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(Level::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(Level::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(Level::ERROR, message);
}

void Logger::log(Level level, const std::string& message) {
    if (level < currentLevel_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = levelToString(level);
    std::string logMessage = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    if (consoleOutput_) {
        std::cout << logMessage << std::endl;
    }
    
    if (logFile_.is_open()) {
        logFile_ << logMessage << std::endl;
        logFile_.flush();
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::DEBUG: return "DEBUG";
        case Level::INFO: return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

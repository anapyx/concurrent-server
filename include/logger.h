#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static Logger& getInstance();
    
    void setLogFile(const std::string& filename);
    void setLevel(Level level);
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    void log(Level level, const std::string& message);

private:
    Logger() = default;
    ~Logger();
    
    std::string getCurrentTimestamp();
    std::string levelToString(Level level);
    
    std::mutex mutex_;
    std::ofstream logFile_;
    Level currentLevel_ = Level::INFO;
    bool consoleOutput_ = true;
};

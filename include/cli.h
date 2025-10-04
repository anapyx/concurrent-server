#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class CLI {
public:
    CLI(int argc, char* argv[]);
    
    bool hasFlag(const std::string& flag) const;
    std::string getStringOption(const std::string& option, const std::string& defaultValue = "") const;
    int getIntOption(const std::string& option, int defaultValue = 0) const;
    
private:
    std::vector<std::string> args_;
    std::unordered_map<std::string, std::string> options_;
    
    void parseArguments();
};
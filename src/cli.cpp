#include "cli.h"
#include <iostream>

CLI::CLI(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {
        args_.emplace_back(argv[i]);
    }
    parseArguments();
}

bool CLI::hasFlag(const std::string& flag) const {
    for (const auto& arg : args_) {
        if (arg == flag) {
            return true;
        }
    }
    return false;
}

std::string CLI::getStringOption(const std::string& option, const std::string& defaultValue) const {
    auto it = options_.find(option);
    return (it != options_.end()) ? it->second : defaultValue;
}

int CLI::getIntOption(const std::string& option, int defaultValue) const {
    auto it = options_.find(option);
    if (it != options_.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

void CLI::parseArguments() {
    for (size_t i = 1; i < args_.size(); ++i) {
        const std::string& arg = args_[i];
        
        if ((arg.length() >= 2 && arg.substr(0, 2) == "--") || 
            (arg.length() >= 1 && arg[0] == '-')) {
            if (i + 1 < args_.size() && args_[i + 1][0] != '-') {
                options_[arg] = args_[i + 1];
                ++i; // Skip next argument as it's the value
            } else {
                options_[arg] = "true"; // Flag without value
            }
        }
    }
}
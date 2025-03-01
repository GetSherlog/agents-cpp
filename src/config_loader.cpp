#include <agents-cpp/config_loader.h>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace agents {

ConfigLoader::ConfigLoader() {
    // Default location for the .env file
    envFilePath = findEnvFile();
    loadEnvFile();
}

ConfigLoader::ConfigLoader(const std::string& customPath) {
    envFilePath = customPath;
    loadEnvFile();
}

std::string ConfigLoader::findEnvFile() {
    // Try looking in standard locations
    std::vector<std::string> paths = {
        ".env",
        "../.env",
        "../../.env",
        std::string(getenv("HOME") ? getenv("HOME") : "") + "/.agents-cpp/.env"
    };
    
    for (const auto& path : paths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    
    return ".env";  // Default to local .env, even if it doesn't exist yet
}

void ConfigLoader::loadEnvFile() {
    std::ifstream file(envFilePath);
    if (!file.is_open()) {
        return;  // File doesn't exist, that's okay
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Find the equals sign
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // Remove quotes if present
            if (value.size() >= 2 && 
                ((value[0] == '"' && value[value.size()-1] == '"') || 
                 (value[0] == '\'' && value[value.size()-1] == '\''))) {
                value = value.substr(1, value.size() - 2);
            }
            
            // Store in our config map
            config[key] = value;
        }
    }
}

std::string ConfigLoader::get(const std::string& key, const std::string& defaultValue) const {
    auto it = config.find(key);
    if (it != config.end()) {
        return it->second;
    }
    
    // If not in config, check environment variables
    const char* envValue = std::getenv(key.c_str());
    if (envValue) {
        return envValue;
    }
    
    return defaultValue;
}

bool ConfigLoader::has(const std::string& key) const {
    if (config.find(key) != config.end()) {
        return true;
    }
    
    // Also check environment variables
    return std::getenv(key.c_str()) != nullptr;
}

// Static instance for global access
ConfigLoader& ConfigLoader::getInstance() {
    static ConfigLoader instance;
    return instance;
}

}  // namespace agents 
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>

class ConfigManager {
public:
    static ConfigManager& getInstance();

    void loadFromYAML(const std::string& filepath);

    std::string getString(const std::string& key, const std::string& def = "") const;
    int         getInt(const std::string& key, int def = 0) const;

private:
    ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    void parseLine(const std::string& line, std::string& prefix, int& indent);
    std::string trim(const std::string& s) const;

    std::unordered_map<std::string, std::string> data_;
};

#endif // CONFIG_H

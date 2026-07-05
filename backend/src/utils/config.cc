#include "config.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

std::string ConfigManager::trim(const std::string& s) const {
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t')) ++start;
    size_t end = s.size();
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t')) --end;
    return s.substr(start, end - start);
}

void ConfigManager::parseLine(const std::string& line, std::string& prefix, int& indent) {
    std::string trimmed = trim(line);
    if (trimmed.empty() || trimmed[0] == '#') return;

    int currentIndent = 0;
    for (char c : line) {
        if (c == ' ') ++currentIndent;
        else break;
    }

    while (!prefix.empty() && currentIndent <= indent) {
        auto pos = prefix.rfind('.');
        if (pos == std::string::npos) {
            prefix.clear();
            break;
        }
        indent -= 2;
        prefix = prefix.substr(0, pos);
    }

    auto colon = trimmed.find(':');
    if (colon == std::string::npos) return;

    std::string key = trim(trimmed.substr(0, colon));
    std::string value = trim(trimmed.substr(colon + 1));

    if (value.empty()) {
        prefix = prefix.empty() ? key : prefix + "." + key;
        indent = currentIndent;
        return;
    }

    if (value.front() == '"' && value.back() == '"')
        value = value.substr(1, value.size() - 2);

    std::string fullKey = prefix.empty() ? key : prefix + "." + key;
    data_[fullKey] = value;
}

void ConfigManager::loadFromYAML(const std::string& filepath) {
    data_.clear();
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file: " + filepath);

    std::string line;
    std::string prefix;
    int indent = -1;

    while (std::getline(file, line)) {
        parseLine(line, prefix, indent);
    }
}

std::string ConfigManager::getString(const std::string& key, const std::string& def) const {
    auto it = data_.find(key);
    return it != data_.end() ? it->second : def;
}

int ConfigManager::getInt(const std::string& key, int def) const {
    auto it = data_.find(key);
    if (it == data_.end()) return def;
    try {
        return std::stoi(it->second);
    } catch (...) {
        return def;
    }
}

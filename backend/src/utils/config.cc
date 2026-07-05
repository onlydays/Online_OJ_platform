#include "config.h"
#include <fstream>
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

int ConfigManager::countIndent(const std::string& line) const {
    int n = 0;
    for (char c : line) {
        if (c == ' ') ++n;
        else break;
    }
    return n;
}

void ConfigManager::loadFromYAML(const std::string& filepath) {
    data_.clear();

    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file: " + filepath);

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    std::vector<std::string> prefixStack;
    std::vector<int> indentStack;

    for (size_t i = 0; i < lines.size(); ++i) {
        std::string trimmed = trim(lines[i]);
        if (trimmed.empty() || trimmed[0] == '#') continue;

        int curIndent = countIndent(lines[i]);

        while (!indentStack.empty() && curIndent <= indentStack.back()) {
            prefixStack.pop_back();
            indentStack.pop_back();
        }

        auto colon = trimmed.find(':');
        if (colon == std::string::npos) continue;

        std::string key = trim(trimmed.substr(0, colon));
        std::string value = trim(trimmed.substr(colon + 1));

        if (!value.empty() && value.front() == '"' && value.back() == '"')
            value = value.substr(1, value.size() - 2);

        std::string fullKey;
        for (const auto& p : prefixStack) {
            fullKey += p + ".";
        }
        fullKey += key;

        if (value.empty() && i + 1 < lines.size() && countIndent(lines[i + 1]) > curIndent) {
            prefixStack.push_back(key);
            indentStack.push_back(curIndent);
        } else {
            data_[fullKey] = value;
        }
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
        size_t pos = 0;
        int val = std::stoi(it->second, &pos);
        if (pos != it->second.size()) return def;
        return val;
    } catch (...) {
        return def;
    }
}

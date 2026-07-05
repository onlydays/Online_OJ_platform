#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace config {

struct DatabaseConfig {
    std::string host;
    int port;
    std::string user;
    std::string password;
    std::string database;
};

struct ServerConfig {
    int port;
    std::string static_dir;
};

struct JudgeConfig {
    std::string docker_image;
    std::string code_storage_path;
    int max_code_size;
};

struct AuthConfig {
    std::string jwt_secret;
    int jwt_expire_seconds;
};

void load(const std::string& config_file = "");

const DatabaseConfig& database();
const ServerConfig& server();
const JudgeConfig& judge();
const AuthConfig& auth();

} // namespace config

#endif // CONFIG_H

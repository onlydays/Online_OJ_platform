#include "config.h"
#include <cstdlib>

namespace config {

static DatabaseConfig g_database;
static ServerConfig    g_server;
static JudgeConfig     g_judge;
static AuthConfig      g_auth;

void load(const std::string& config_file) {
    (void)config_file;

    g_database = {
        "127.0.0.1",
        3306,
        "root",
        "2791830200Zby..",
        "oj_platform"
    };

    g_server = {
        8080,
        "../frontend"
    };

    g_judge = {
        "oj-judge:latest",
        "./data/submissions",
        65536
    };

    g_auth = {
        "oj-platform-jwt-secret-key-2026",
        86400
    };
}

const DatabaseConfig& database() { return g_database; }
const ServerConfig&   server()   { return g_server;   }
const JudgeConfig&    judge()    { return g_judge;    }
const AuthConfig&     auth()     { return g_auth;     }

} // namespace config

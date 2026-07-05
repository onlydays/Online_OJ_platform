#include <httplib.h>
#include "router.h"
#include "utils/config.h"
#include "utils/logger.h"
#include "db/connection_pool.h"
#include "judge/judge_manager.h"

int main() {
    LOG_INFO("Online OJ Platform starting...");

    // Load config
    auto& cfg = ConfigManager::getInstance();
    cfg.loadFromYAML("config.yaml");

    // Init database pool
    auto& db = ConnectionPool::getInstance();
    db.init(
        cfg.getString("database.host", "127.0.0.1"),
        cfg.getInt("database.port", 3306),
        cfg.getString("database.username", "root"),
        cfg.getString("database.password", ""),
        cfg.getString("database.database", "oj_platform"),
        cfg.getInt("database.max_connections", 10)
    );

    // Start judge manager
    JudgeManager::getInstance().start(
        cfg.getInt("server.thread_pool_size", 4));

    // HTTP server
    httplib::Server server;

    // Static files
    std::string staticDir = cfg.getString("server.static_dir", "../frontend");
    server.set_mount_point("/", staticDir);

    // API routes
    registerAllRoutes(server);

    // Exception handler
    server.set_exception_handler([](const auto& req, auto& res, std::exception_ptr ep) {
        try {
            std::rethrow_exception(ep);
        } catch (std::exception& e) {
            LOG_ERROR("Unhandled exception: " << e.what());
            res.status = 500;
            res.set_content(R"({"code":500,"message":"Internal server error"})",
                            "application/json");
        }
    });

    int port = cfg.getInt("server.port", 8080);
    std::string host = cfg.getString("server.host", "0.0.0.0");

    LOG_INFO("Server listening on " << host << ":" << port);
    server.listen(host.c_str(), port);

    JudgeManager::getInstance().stop();
    return 0;
}

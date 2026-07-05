#include "user_handler.h"
#include "auth.h"
#include "models/user.h"
#include "utils/json.h"
#include "utils/logger.h"

#include <httplib.h>

static std::optional<int> getUserIdFromRequest(const httplib::Request& req) {
    auto authHeader = req.get_header_value("Authorization");
    if (authHeader.empty() || authHeader.size() < 8) return std::nullopt;
    std::string token = authHeader.substr(7);
    return auth::verifyToken(token);
}

void registerUserRoutes(httplib::Server& server) {
    server.Get("/api/users/me", [](const httplib::Request& req,
                                     httplib::Response& res) {
        auto userId = getUserIdFromRequest(req);
        if (!userId.has_value()) {
            res.status = 401;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(401, "Unauthorized")),
                "application/json");
            return;
        }

        auto user = UserModel::getInstance().findById(*userId);
        if (!user.has_value()) {
            res.status = 404;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(404, "User not found")),
                "application/json");
            return;
        }

        Json data;
        data["id"] = user->id;
        data["username"] = user->username;
        data["role"] = user->role;
        data["created_at"] = user->createdAt;

        res.status = 200;
        res.set_content(json_utils::serialize(json_utils::successResponse(data)),
                        "application/json");
    });

    server.Get("/api/users/me/stats", [](const httplib::Request& req,
                                           httplib::Response& res) {
        auto userId = getUserIdFromRequest(req);
        if (!userId.has_value()) {
            res.status = 401;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(401, "Unauthorized")),
                "application/json");
            return;
        }

        Json stats;
        stats["total_submissions"] = 0;
        stats["accepted"] = 0;
        stats["ac_rate"] = 0.0;
        stats["problems_solved"] = 0;

        res.status = 200;
        res.set_content(json_utils::serialize(json_utils::successResponse(stats)),
                        "application/json");
    });
}

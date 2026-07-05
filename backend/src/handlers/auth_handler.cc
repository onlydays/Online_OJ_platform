#include "auth_handler.h"
#include "auth.h"
#include "models/user.h"
#include "utils/json.h"
#include "utils/logger.h"

#include <httplib.h>

void registerAuthRoutes(httplib::Server& server) {
    server.Post("/api/auth/register", [](const httplib::Request& req,
                                           httplib::Response& res) {
        try {
            Json body = json_utils::deserialize(req.body);

            std::string username = body["username"].get<std::string>();
            std::string password = body["password"].get<std::string>();

            if (username.empty() || password.empty()) {
                res.status = 400;
                res.set_content(
                    json_utils::serialize(json_utils::errorResponse(400, "Username and password required")),
                    "application/json");
                return;
            }

            if (username.size() > 50) {
                res.status = 400;
                res.set_content(
                    json_utils::serialize(json_utils::errorResponse(400, "Username too long")),
                    "application/json");
                return;
            }

            auto existing = UserModel::getInstance().findByUsername(username);
            if (existing.has_value()) {
                res.status = 409;
                res.set_content(
                    json_utils::serialize(json_utils::errorResponse(409, "Username already exists")),
                    "application/json");
                return;
            }

            std::string hash = auth::sha256(password);
            int userId = UserModel::getInstance().create(username, hash);

            Json data;
            data["id"] = userId;
            data["username"] = username;
            data["role"] = "user";

            res.status = 201;
            res.set_content(json_utils::serialize(json_utils::successResponse(data)),
                            "application/json");
        } catch (const std::exception& e) {
            LOG_ERROR("Register error: " << e.what());
            res.status = 400;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(400, "Invalid request")),
                "application/json");
        }
    });

    server.Post("/api/auth/login", [](const httplib::Request& req,
                                        httplib::Response& res) {
        try {
            Json body = json_utils::deserialize(req.body);

            std::string username = body["username"].get<std::string>();
            std::string password = body["password"].get<std::string>();

            auto user = UserModel::getInstance().findByUsername(username);
            if (!user.has_value()) {
                res.status = 401;
                res.set_content(
                    json_utils::serialize(json_utils::errorResponse(401, "Invalid username or password")),
                    "application/json");
                return;
            }

            std::string hash = auth::sha256(password);
            if (hash != user->passwordHash) {
                res.status = 401;
                res.set_content(
                    json_utils::serialize(json_utils::errorResponse(401, "Invalid username or password")),
                    "application/json");
                return;
            }

            std::string token = auth::generateToken(user->id, user->username, user->role);

            Json data;
            data["token"] = token;
            Json userJson;
            userJson["id"] = user->id;
            userJson["username"] = user->username;
            userJson["role"] = user->role;
            data["user"] = userJson;

            res.status = 200;
            res.set_content(json_utils::serialize(json_utils::successResponse(data)),
                            "application/json");
        } catch (const std::exception& e) {
            LOG_ERROR("Login error: " << e.what());
            res.status = 400;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(400, "Invalid request")),
                "application/json");
        }
    });
}

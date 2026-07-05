#include "problem_handler.h"
#include "auth.h"
#include "models/problem.h"
#include "models/testcase.h"
#include "utils/json.h"
#include "utils/logger.h"

#include <httplib.h>

static bool isAdmin(const httplib::Request& req) {
    auto auth = req.get_header_value("Authorization");
    if (auth.size() < 8) return false;
    return auth::extractRole(auth.substr(7)) == "admin";
}

static std::optional<int> getUserId(const httplib::Request& req) {
    auto auth = req.get_header_value("Authorization");
    if (auth.size() < 8) return std::nullopt;
    return auth::verifyToken(auth.substr(7));
}

void registerProblemRoutes(httplib::Server& server) {
    // === 公开接口 ===

    server.Get("/api/problems", [](const httplib::Request& req,
                                     httplib::Response& res) {
        std::string difficulty;
        if (req.has_param("difficulty")) {
            difficulty = req.get_param_value("difficulty");
        }
        int page = 1, size = 20;
        if (req.has_param("page")) page = std::stoi(req.get_param_value("page"));
        if (req.has_param("size")) size = std::stoi(req.get_param_value("size"));

        auto problems = ProblemModel::getInstance().list(difficulty, page, size);
        int total = ProblemModel::getInstance().total(difficulty);

        Json arr = Json::array();
        for (auto& p : problems) {
            Json item;
            item["id"] = p.id;
            item["title"] = p.title;
            item["difficulty"] = p.difficulty;
            item["ac_rate"] = p.acRate;
            arr.push_back(item);
        }

        Json resp;
        resp["code"] = 0;
        resp["total"] = total;
        resp["problems"] = arr;

        res.status = 200;
        res.set_content(json_utils::serialize(resp), "application/json");
    });

    server.Get(R"(/api/problems/(\d+))", [](const httplib::Request& req,
                                              httplib::Response& res) {
        int id = std::stoi(req.matches[1]);
        auto problem = ProblemModel::getInstance().findById(id);
        if (!problem.has_value()) {
            res.status = 404;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(404, "Problem not found")),
                "application/json");
            return;
        }

        auto samples = TestCaseModel::getInstance().findByProblemId(id, true);

        Json data;
        data["id"] = problem->id;
        data["title"] = problem->title;
        data["description"] = problem->description;
        data["difficulty"] = problem->difficulty;
        data["time_limit_ms"] = problem->timeLimitMs;
        data["memory_limit_kb"] = problem->memoryLimitKb;
        data["ac_rate"] = problem->acRate;

        Json sampleArr = Json::array();
        for (auto& tc : samples) {
            Json item;
            item["input"] = tc.input;
            item["expected_output"] = tc.expectedOutput;
            sampleArr.push_back(item);
        }
        data["sample_testcases"] = sampleArr;

        res.status = 200;
        res.set_content(json_utils::serialize(json_utils::successResponse(data)),
                        "application/json");
    });

    // === 管理员接口 ===

    server.Post("/api/problems", [](const httplib::Request& req,
                                      httplib::Response& res) {
        if (!isAdmin(req)) {
            res.status = 403;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(403, "Admin only")),
                "application/json");
            return;
        }

        try {
            Json body = json_utils::deserialize(req.body);
            Problem p;
            p.title = body["title"].get<std::string>();
            p.description = body["description"].get<std::string>();
            p.difficulty = body.value("difficulty", "medium");
            p.timeLimitMs = body.value("time_limit_ms", 1000);
            p.memoryLimitKb = body.value("memory_limit_kb", 262144);
            p.createdBy = getUserId(req).value_or(0);

            int id = ProblemModel::getInstance().create(p);

            Json data;
            data["id"] = id;
            data["title"] = p.title;

            res.status = 201;
            res.set_content(json_utils::serialize(json_utils::successResponse(data)),
                            "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(400, e.what())),
                "application/json");
        }
    });

    server.Put(R"(/api/problems/(\d+))", [](const httplib::Request& req,
                                             httplib::Response& res) {
        if (!isAdmin(req)) {
            res.status = 403;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(403, "Admin only")),
                "application/json");
            return;
        }

        try {
            int id = std::stoi(req.matches[1]);
            Json body = json_utils::deserialize(req.body);
            Problem p;
            p.title = body["title"].get<std::string>();
            p.description = body["description"].get<std::string>();
            p.difficulty = body.value("difficulty", "medium");
            p.timeLimitMs = body.value("time_limit_ms", 1000);
            p.memoryLimitKb = body.value("memory_limit_kb", 262144);

            bool ok = ProblemModel::getInstance().update(id, p);
            if (!ok) {
                res.status = 404;
                res.set_content(
                    json_utils::serialize(json_utils::errorResponse(404, "Problem not found")),
                    "application/json");
                return;
            }

            res.status = 200;
            res.set_content(
                json_utils::serialize(json_utils::successResponse("message", "updated")),
                "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(400, e.what())),
                "application/json");
        }
    });

    server.Delete(R"(/api/problems/(\d+))", [](const httplib::Request& req,
                                                httplib::Response& res) {
        if (!isAdmin(req)) {
            res.status = 403;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(403, "Admin only")),
                "application/json");
            return;
        }

        int id = std::stoi(req.matches[1]);
        TestCaseModel::getInstance().removeByProblemId(id);
        ProblemModel::getInstance().remove(id);

        res.status = 200;
        res.set_content(
            json_utils::serialize(json_utils::successResponse("message", "deleted")),
            "application/json");
    });

    // === 测试用例管理 (Admin) ===

    server.Get(R"(/api/problems/(\d+)/testcases)", [](const httplib::Request& req,
                                                        httplib::Response& res) {
        if (!isAdmin(req)) {
            res.status = 403;
            return;
        }

        int problemId = std::stoi(req.matches[1]);
        auto testcases = TestCaseModel::getInstance().findAllByProblemId(problemId);

        Json arr = Json::array();
        for (auto& tc : testcases) {
            Json item;
            item["id"] = tc.id;
            item["input"] = tc.input;
            item["expected_output"] = tc.expectedOutput;
            item["is_sample"] = tc.isSample;
            item["order_index"] = tc.orderIndex;
            arr.push_back(item);
        }

        res.status = 200;
        res.set_content(
            json_utils::serialize(json_utils::arrayResponse("testcases", arr)),
            "application/json");
    });

    server.Post(R"(/api/problems/(\d+)/testcases)", [](const httplib::Request& req,
                                                         httplib::Response& res) {
        if (!isAdmin(req)) {
            res.status = 403;
            return;
        }

        try {
            int problemId = std::stoi(req.matches[1]);
            Json body = json_utils::deserialize(req.body);
            auto tcs = body["testcases"];

            int count = 0;
            for (size_t i = 0; i < tcs.size(); ++i) {
                auto& tc = tcs[i];
                TestCaseModel::getInstance().add(
                    problemId,
                    tc["input"].get<std::string>(),
                    tc["expected_output"].get<std::string>(),
                    tc.value("is_sample", false),
                    static_cast<int>(i));
                ++count;
            }

            res.status = 201;
            Json data;
            data["count"] = count;
            res.set_content(json_utils::serialize(json_utils::successResponse(data)),
                            "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(400, e.what())),
                "application/json");
        }
    });

    server.Delete(R"(/api/testcases/(\d+))", [](const httplib::Request& req,
                                                  httplib::Response& res) {
        if (!isAdmin(req)) {
            res.status = 403;
            return;
        }

        int id = std::stoi(req.matches[1]);
        TestCaseModel::getInstance().remove(id);

        res.status = 200;
        res.set_content(
            json_utils::serialize(json_utils::successResponse("message", "deleted")),
            "application/json");
    });
}

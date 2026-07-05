#include "submission_handler.h"
#include "auth.h"
#include "models/submission.h"
#include "models/problem.h"
#include "models/testcase.h"
#include "judge/judge_manager.h"
#include "judge/judge_result.h"
#include "utils/json.h"
#include "utils/config.h"
#include "utils/logger.h"

#include <httplib.h>
#include <fstream>

static std::optional<int> getUserId(const httplib::Request& req) {
    auto auth = req.get_header_value("Authorization");
    if (auth.size() < 8) return std::nullopt;
    return auth::verifyToken(auth.substr(7));
}

static void onJudgeComplete(int submissionId, const std::string& outputJson) {
    auto result = parseJudgeOutput(outputJson);
    SubmissionModel::getInstance().updateResult(
        submissionId, result.status, result.timeUsedMs,
        result.memoryUsedKb, result.compileError);

    for (auto& tc : result.results) {
        SubmissionModel::getInstance().saveDetail(
            submissionId, 0, tc.status, tc.timeUsedMs,
            tc.memoryUsedKb, tc.actual);
    }
    LOG_INFO("Submission " << submissionId << " judged: " << result.status);
}

void registerSubmissionRoutes(httplib::Server& server) {
    server.Post("/api/submissions", [](const httplib::Request& req,
                                        httplib::Response& res) {
        auto uid = getUserId(req);
        if (!uid.has_value()) {
            res.status = 401;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(401, "Unauthorized")),
                "application/json");
            return;
        }

        try {
            Json body = json_utils::deserialize(req.body);
            int problemId = body["problem_id"].get<int>();
            std::string code = body["code"].get<std::string>();

            if (code.empty()) {
                res.status = 400;
                res.set_content(
                    json_utils::serialize(json_utils::errorResponse(400, "Code cannot be empty")),
                    "application/json");
                return;
            }

            if (code.size() > 65536) {
                res.status = 413;
                res.set_content(
                    json_utils::serialize(json_utils::errorResponse(413, "Code too large")),
                    "application/json");
                return;
            }

            auto problem = ProblemModel::getInstance().findById(problemId);
            if (!problem.has_value()) {
                res.status = 404;
                res.set_content(
                    json_utils::serialize(json_utils::errorResponse(404, "Problem not found")),
                    "application/json");
                return;
            }

            auto& cfg = ConfigManager::getInstance();
            std::string codePath = cfg.getString("judge.code_storage_path", "./data/submissions");
            std::string fileName = std::to_string(*uid) + "_" +
                                   std::to_string(time(nullptr)) + "_" +
                                   std::to_string(problemId) + ".cpp";
            std::string fullPath = codePath + "/" + fileName;

            int subId = SubmissionModel::getInstance().create(
                *uid, problemId, fullPath);

            auto testcases = TestCaseModel::getInstance().findAllByProblemId(problemId);

            Json tcJson = Json::array();
            for (auto& tc : testcases) {
                Json item;
                item["input"] = tc.input;
                item["expected"] = tc.expectedOutput;
                tcJson.push_back(item);
            }

            JudgeRequest jreq;
            jreq.submissionId = subId;
            jreq.code = code;
            jreq.language = "cpp";
            jreq.timeLimitMs = problem->timeLimitMs;
            jreq.memoryLimitKb = problem->memoryLimitKb;
            jreq.testcasesJson = json_utils::serialize(tcJson);

            JudgeManager::getInstance().submit(jreq, onJudgeComplete);

            Json resp;
            resp["id"] = subId;
            resp["status"] = "pending";
            resp["created_at"] = "";

            res.status = 201;
            res.set_content(
                json_utils::serialize(json_utils::successResponse(resp)),
                "application/json");
        } catch (const std::exception& e) {
            LOG_ERROR("Submit error: " << e.what());
            res.status = 400;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(400, "Invalid request")),
                "application/json");
        }
    });

    server.Get("/api/submissions", [](const httplib::Request& req,
                                       httplib::Response& res) {
        auto uid = getUserId(req);
        if (!uid.has_value()) {
            res.status = 401;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(401, "Unauthorized")),
                "application/json");
            return;
        }

        int page = req.has_param("page") ? std::stoi(req.get_param_value("page")) : 1;
        int size = req.has_param("size") ? std::stoi(req.get_param_value("size")) : 20;

        auto subs = SubmissionModel::getInstance().listByUser(*uid, page, size);

        Json arr = Json::array();
        for (auto& s : subs) {
            Json item;
            item["id"] = s.id;
            item["problem_id"] = s.problemId;
            item["status"] = s.status;
            item["time_used_ms"] = s.timeUsedMs;
            item["memory_used_kb"] = s.memoryUsedKb;
            item["created_at"] = s.createdAt;
            arr.push_back(item);
        }

        res.status = 200;
        res.set_content(
            json_utils::serialize(json_utils::arrayResponse("submissions", arr)),
            "application/json");
    });

    server.Get(R"(/api/submissions/(\d+))", [](const httplib::Request& req,
                                                 httplib::Response& res) {
        int id = std::stoi(req.matches[1]);
        auto sub = SubmissionModel::getInstance().findById(id);
        if (!sub.has_value()) {
            res.status = 404;
            res.set_content(
                json_utils::serialize(json_utils::errorResponse(404, "Submission not found")),
                "application/json");
            return;
        }

        Json data;
        data["id"] = sub->id;
        data["problem_id"] = sub->problemId;
        data["status"] = sub->status;
        data["time_used_ms"] = sub->timeUsedMs;
        data["memory_used_kb"] = sub->memoryUsedKb;
        data["compile_error"] = sub->compileError;
        data["created_at"] = sub->createdAt;

        auto details = SubmissionModel::getInstance().getDetails(id);
        Json results = Json::array();
        for (auto& d : details) {
            Json item;
            item["status"] = d.status;
            item["time_used_ms"] = d.timeUsedMs;
            item["memory_used_kb"] = d.memoryUsedKb;
            item["actual_output"] = d.actualOutput;
            results.push_back(item);
        }
        data["results"] = results;

        res.status = 200;
        res.set_content(json_utils::serialize(json_utils::successResponse(data)),
                        "application/json");
    });
}

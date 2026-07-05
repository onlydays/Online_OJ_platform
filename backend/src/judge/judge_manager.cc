#include "judge_manager.h"
#include "utils/config.h"
#include "utils/json.h"
#include "utils/logger.h"

#include <cstdio>
#include <fstream>
#include <array>
#include <memory>
#include <cstdlib>
#include <unistd.h>

// ----  ThreadPool ----

ThreadPool::ThreadPool(size_t threads) : stop_(false) {
    for (size_t i = 0; i < threads; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cond_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                    if (stop_ && tasks_.empty()) return;
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cond_.notify_all();
    for (auto& w : workers_) {
        if (w.joinable()) w.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push(std::move(task));
    }
    cond_.notify_one();
}

// ----  JudgeManager ----

JudgeManager& JudgeManager::getInstance() {
    static JudgeManager instance;
    return instance;
}

void JudgeManager::start(size_t workerCount) {
    if (pool_) return;
    pool_ = std::make_unique<ThreadPool>(workerCount);
    LOG_INFO("Judge manager started with " << workerCount << " workers");
}

void JudgeManager::stop() {
    pool_.reset();
    LOG_INFO("Judge manager stopped");
}

void JudgeManager::submit(const JudgeRequest& req, JudgeCallback callback) {
    if (!pool_) {
        LOG_ERROR("Judge manager not started");
        return;
    }

    pool_->enqueue([this, req, callback]() {
        LOG_INFO("Judging submission " << req.submissionId);
        std::string output = runJudgeContainer(req);
        callback(req.submissionId, output);
    });
}

std::string JudgeManager::runJudgeContainer(const JudgeRequest& req) {
    auto& cfg = ConfigManager::getInstance();
    std::string image = cfg.getString("judge.docker_image", "oj-judge:latest");
    std::string codePath = cfg.getString("judge.code_storage_path", "./data/submissions");

    // ----  Write code to temp file ----
    std::string codeFile = codePath + "/" + std::to_string(req.submissionId) + ".cpp";
    std::ofstream out(codeFile);
    if (!out) {
        Json err;
        err["status"] = "runtime_error";
        err["error"] = "Failed to write code file";
        return json_utils::serialize(err);
    }
    out << req.code;
    out.close();

    // ----  Build testcases JSON payload ----
    Json tcArray;
    try {
        tcArray = json_utils::deserialize(req.testcasesJson);
    } catch (...) {
        tcArray = Json::array();
    }

    Json payload;
    payload["code"] = req.code;
    payload["time_limit_ms"] = req.timeLimitMs;
    payload["memory_limit_kb"] = req.memoryLimitKb;
    payload["testcases"] = tcArray;

    std::string payloadFile = codePath + "/payload_" + std::to_string(req.submissionId) + ".json";
    std::ofstream pout(payloadFile);
    pout << json_utils::serialize(payload);
    pout.close();

    // ----  Run Docker container ----
    std::string cmd = "docker run --rm --network=none --memory="
                      + std::to_string(req.memoryLimitKb * 2) + "k"
                      + " --cpus=1"
                      + " -v " + codePath + ":/data:ro"
                      + " " + image
                      + " < " + payloadFile + " 2>/dev/null";

    std::array<char, 65536> buffer;
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        Json err;
        err["status"] = "runtime_error";
        err["error"] = "Failed to start judge container";
        return json_utils::serialize(err);
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    int exitCode = pclose(pipe);

    // Cleanup temp files
    remove(codeFile.c_str());
    remove(payloadFile.c_str());

    if (!result.empty()) {
        return result;
    }

    // Docker failure fallback
    Json fallback;
    fallback["status"] = "runtime_error";
    fallback["time_used_ms"] = 0;
    fallback["memory_used_kb"] = 0;
    fallback["error"] = "Judge container returned no output (exit " + std::to_string(exitCode) + ")";
    return json_utils::serialize(fallback);
}

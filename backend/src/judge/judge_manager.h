#ifndef JUDGE_MANAGER_H
#define JUDGE_MANAGER_H

#include <functional>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(size_t threads);
    ~ThreadPool();

    void enqueue(std::function<void()> task);

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> stop_;
};

struct JudgeRequest {
    int submissionId;
    std::string code;
    std::string language;
    int timeLimitMs;
    int memoryLimitKb;
    std::string testcasesJson;
};

using JudgeCallback = std::function<void(int submissionId, const std::string& outputJson)>;

class JudgeManager {
public:
    static JudgeManager& getInstance();

    void start(size_t workerCount = 2);
    void stop();

    void submit(const JudgeRequest& req, JudgeCallback callback);

private:
    JudgeManager() = default;
    void workerLoop();

    std::string runJudgeContainer(const JudgeRequest& req);

    std::unique_ptr<ThreadPool> pool_;
};

#endif

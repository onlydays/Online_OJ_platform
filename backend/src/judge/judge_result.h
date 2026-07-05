#ifndef JUDGE_RESULT_H
#define JUDGE_RESULT_H

#include <string>
#include <vector>

struct TestCaseResult {
    std::string status;
    int timeUsedMs;
    int memoryUsedKb;
    std::string input;
    std::string expected;
    std::string actual;
};

struct JudgeResult {
    std::string status;
    int timeUsedMs;
    int memoryUsedKb;
    std::string compileError;
    std::vector<TestCaseResult> results;
};

JudgeResult parseJudgeOutput(const std::string& json);

#endif

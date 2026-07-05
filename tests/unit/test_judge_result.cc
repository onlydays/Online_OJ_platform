#include <gtest/gtest.h>
#include "judge/judge_result.h"
#include "utils/json.h"

TEST(JudgeResultTest, ParseAccepted) {
    std::string json = R"({
        "status": "accepted",
        "time_used_ms": 42,
        "memory_used_kb": 4096,
        "results": [
            {"status": "accepted", "time_used_ms": 10, "memory_used_kb": 1024},
            {"status": "accepted", "time_used_ms": 32, "memory_used_kb": 3072}
        ]
    })";

    auto result = parseJudgeOutput(json);
    EXPECT_EQ(result.status, "accepted");
    EXPECT_EQ(result.timeUsedMs, 42);
    EXPECT_EQ(result.memoryUsedKb, 4096);
    EXPECT_EQ(result.results.size(), 2);
    EXPECT_EQ(result.results[0].status, "accepted");
    EXPECT_EQ(result.results[0].timeUsedMs, 10);
}

TEST(JudgeResultTest, ParseCompilationError) {
    std::string json = R"({
        "status": "compilation_error",
        "error": "syntax error at line 5",
        "time_used_ms": 0,
        "memory_used_kb": 0
    })";

    auto result = parseJudgeOutput(json);
    EXPECT_EQ(result.status, "compilation_error");
    EXPECT_EQ(result.compileError, "syntax error at line 5");
    EXPECT_EQ(result.results.size(), 0);
}

TEST(JudgeResultTest, ParseWrongAnswer) {
    std::string json = R"({
        "status": "wrong_answer",
        "time_used_ms": 15,
        "memory_used_kb": 2048,
        "results": [
            {"status": "accepted", "time_used_ms": 5, "memory_used_kb": 1024},
            {"status": "wrong_answer", "time_used_ms": 10, "memory_used_kb": 1024,
             "input": "1 2", "expected": "3", "actual": "4"}
        ]
    })";

    auto result = parseJudgeOutput(json);
    EXPECT_EQ(result.status, "wrong_answer");
    EXPECT_EQ(result.results[1].input, "1 2");
    EXPECT_EQ(result.results[1].expected, "3");
    EXPECT_EQ(result.results[1].actual, "4");
}

TEST(JudgeResultTest, ParsePartialStates) {
    std::string json = R"({
        "status": "runtime_error",
        "results": [
            {"status": "accepted"},
            {"status": "time_limit_exceeded"},
            {"status": "runtime_error"}
        ]
    })";

    auto result = parseJudgeOutput(json);
    EXPECT_EQ(result.status, "runtime_error");
    EXPECT_EQ(result.results.size(), 3);
    EXPECT_EQ(result.results[1].status, "time_limit_exceeded");
}

TEST(JudgeResultTest, ParseInvalidJson) {
    auto result = parseJudgeOutput("not valid json at all");
    EXPECT_EQ(result.status, "runtime_error");
    EXPECT_TRUE(result.compileError.find("Failed") != std::string::npos);
}

TEST(JudgeResultTest, ParseEmptyJson) {
    auto result = parseJudgeOutput("{}");
    EXPECT_EQ(result.status, "runtime_error");
    EXPECT_EQ(result.results.size(), 0);
}

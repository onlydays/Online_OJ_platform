#include "judge_result.h"
#include "utils/json.h"

JudgeResult parseJudgeOutput(const std::string& json) {
    JudgeResult result;
    try {
        Json j = json_utils::deserialize(json);

        result.status = j.value("status", "runtime_error");
        result.timeUsedMs = j.value("time_used_ms", 0);
        result.memoryUsedKb = j.value("memory_used_kb", 0);

        if (j.contains("error")) {
            result.compileError = j["error"].get<std::string>();
        }

        if (j.contains("results") && j["results"].is_array()) {
            for (auto& item : j["results"]) {
                TestCaseResult tc;
                tc.status = item.value("status", "runtime_error");
                tc.timeUsedMs = item.value("time_used_ms", 0);
                tc.memoryUsedKb = item.value("memory_used_kb", 0);
                tc.input = item.value("input", "");
                tc.expected = item.value("expected", "");
                tc.actual = item.value("actual", "");
                result.results.push_back(tc);
            }
        }
    } catch (const std::exception&) {
        result.status = "runtime_error";
        result.compileError = "Failed to parse judge output";
    }
    return result;
}

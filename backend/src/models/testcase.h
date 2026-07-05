#ifndef TESTCASE_MODEL_H
#define TESTCASE_MODEL_H

#include <string>
#include <vector>
#include <optional>

struct TestCase {
    int id;
    int problemId;
    std::string input;
    std::string expectedOutput;
    bool isSample;
    int orderIndex;
    std::string createdAt;
};

class TestCaseModel {
public:
    static TestCaseModel& getInstance();

    std::vector<TestCase> findByProblemId(int problemId, bool samplesOnly = false);
    std::vector<TestCase> findAllByProblemId(int problemId);
    std::optional<TestCase> findById(int id);

    int add(int problemId, const std::string& input,
            const std::string& expectedOutput, bool isSample, int orderIndex);
    bool remove(int id);
    bool removeByProblemId(int problemId);

private:
    TestCaseModel() = default;
};

#endif

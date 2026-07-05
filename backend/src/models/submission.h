#ifndef SUBMISSION_MODEL_H
#define SUBMISSION_MODEL_H

#include <string>
#include <vector>
#include <optional>

struct Submission {
    int id;
    int userId;
    int problemId;
    std::string codePath;
    std::string status;
    int timeUsedMs;
    int memoryUsedKb;
    std::string compileError;
    std::string createdAt;
};

struct SubmissionDetail {
    std::string status;
    int timeUsedMs;
    int memoryUsedKb;
    std::string actualOutput;
};

class SubmissionModel {
public:
    static SubmissionModel& getInstance();

    int create(int userId, int problemId, const std::string& codePath);
    std::optional<Submission> findById(int id);
    std::vector<Submission> listByUser(int userId, int page = 1, int size = 20);

    void updateResult(int id, const std::string& status, int timeMs, int memoryKb,
                      const std::string& compileError);
    void saveDetail(int submissionId, int testCaseId, const std::string& status,
                    int timeMs, int memoryKb, const std::string& actualOutput);
    std::vector<SubmissionDetail> getDetails(int submissionId);

private:
    SubmissionModel() = default;
};

#endif

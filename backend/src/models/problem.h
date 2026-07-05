#ifndef PROBLEM_MODEL_H
#define PROBLEM_MODEL_H

#include <string>
#include <vector>
#include <optional>

struct Problem {
    int id;
    std::string title;
    std::string description;
    std::string difficulty;
    int timeLimitMs;
    int memoryLimitKb;
    int createdBy;
    std::string createdAt;
    std::string updatedAt;
    double acRate;
};

struct ProblemListItem {
    int id;
    std::string title;
    std::string difficulty;
    double acRate;
};

class ProblemModel {
public:
    static ProblemModel& getInstance();

    std::optional<Problem> findById(int id);
    std::vector<ProblemListItem> list(const std::string& difficulty = "",
                                      int page = 1, int size = 20);
    int total(const std::string& difficulty = "");

    int create(const Problem& p);
    bool update(int id, const Problem& p);
    bool remove(int id);

private:
    ProblemModel() = default;
};

#endif

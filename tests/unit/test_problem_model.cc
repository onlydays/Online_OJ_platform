#include <gtest/gtest.h>
#include "models/problem.h"
#include "models/testcase.h"
#include "db/connection_pool.h"

class ProblemModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!ConnectionPool::getInstance().total()) {
            ConnectionPool::getInstance().init(
                "127.0.0.1", 3306, "root", "2791830200Zby..",
                "oj_platform", 3);
        }
    }
};

TEST_F(ProblemModelTest, CreateProblem) {
    Problem p;
    p.title = "Test Problem " + std::to_string(time(nullptr));
    p.description = "Description here";
    p.difficulty = "easy";
    p.timeLimitMs = 1000;
    p.memoryLimitKb = 262144;
    p.createdBy = 1;

    int id = ProblemModel::getInstance().create(p);
    EXPECT_GT(id, 0);
}

TEST_F(ProblemModelTest, FindById) {
    Problem p;
    p.title = "FindById Test " + std::to_string(time(nullptr));
    p.description = "Test desc";
    p.difficulty = "medium";
    p.timeLimitMs = 2000;
    p.memoryLimitKb = 131072;
    p.createdBy = 1;

    int id = ProblemModel::getInstance().create(p);
    auto found = ProblemModel::getInstance().findById(id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->title, p.title);
    EXPECT_EQ(found->difficulty, "medium");
    EXPECT_EQ(found->timeLimitMs, 2000);
}

TEST_F(ProblemModelTest, FindByIdNotFound) {
    auto found = ProblemModel::getInstance().findById(99999999);
    EXPECT_FALSE(found.has_value());
}

TEST_F(ProblemModelTest, ListProblems) {
    auto problems = ProblemModel::getInstance().list("", 1, 50);
    EXPECT_GT(problems.size(), 0);
}

TEST_F(ProblemModelTest, ListWithDifficultyFilter) {
    Problem p;
    p.title = "Easy Filter " + std::to_string(time(nullptr));
    p.description = "test";
    p.difficulty = "easy";
    p.timeLimitMs = 1000;
    p.memoryLimitKb = 262144;
    p.createdBy = 1;
    ProblemModel::getInstance().create(p);

    auto problems = ProblemModel::getInstance().list("easy", 1, 50);
    EXPECT_GT(problems.size(), 0);
    for (auto& item : problems) {
        EXPECT_EQ(item.difficulty, "easy");
    }
}

TEST_F(ProblemModelTest, UpdateProblem) {
    Problem p;
    p.title = "Original Title " + std::to_string(time(nullptr));
    p.description = "Original";
    p.difficulty = "easy";
    p.timeLimitMs = 1000;
    p.memoryLimitKb = 1000;
    p.createdBy = 1;
    int id = ProblemModel::getInstance().create(p);

    Problem updated;
    updated.title = "Updated Title " + std::to_string(time(nullptr));
    updated.description = "Updated";
    updated.difficulty = "hard";
    updated.timeLimitMs = 3000;
    updated.memoryLimitKb = 999;

    bool ok = ProblemModel::getInstance().update(id, updated);
    EXPECT_TRUE(ok);

    auto found = ProblemModel::getInstance().findById(id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->title, updated.title);
    EXPECT_EQ(found->difficulty, "hard");
    EXPECT_EQ(found->timeLimitMs, 3000);
}

TEST_F(ProblemModelTest, DeleteProblem) {
    Problem p;
    p.title = "To Delete " + std::to_string(time(nullptr));
    p.description = "Delete me";
    p.difficulty = "easy";
    p.timeLimitMs = 1000;
    p.memoryLimitKb = 1000;
    p.createdBy = 1;
    int id = ProblemModel::getInstance().create(p);

    bool ok = ProblemModel::getInstance().remove(id);
    EXPECT_TRUE(ok);

    auto found = ProblemModel::getInstance().findById(id);
    EXPECT_FALSE(found.has_value());
}

#include <gtest/gtest.h>
#include "models/submission.h"
#include "models/problem.h"
#include "models/testcase.h"
#include "db/connection_pool.h"

class SubmissionModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!ConnectionPool::getInstance().total()) {
            ConnectionPool::getInstance().init(
                "127.0.0.1", 3306, "root", "2791830200Zby..",
                "oj_platform", 3);
        }
        Problem p;
        p.title = "SubTest_" + std::to_string(time(nullptr));
        p.description = "test";
        p.difficulty = "easy";
        p.timeLimitMs = 1000;
        p.memoryLimitKb = 262144;
        p.createdBy = 1;
        testProblemId_ = ProblemModel::getInstance().create(p);
        testCaseId_ = TestCaseModel::getInstance().add(
            testProblemId_, "1 2", "3", false, 0);
    }

    void TearDown() override {
        TestCaseModel::getInstance().removeByProblemId(testProblemId_);
        ProblemModel::getInstance().remove(testProblemId_);
    }

    int testProblemId_ = 0;
    int testCaseId_ = 0;
};

TEST_F(SubmissionModelTest, CreateSubmission) {
    int id = SubmissionModel::getInstance().create(1, testProblemId_, "/tmp/test.cpp");
    EXPECT_GT(id, 0);
}

TEST_F(SubmissionModelTest, FindById) {
    int id = SubmissionModel::getInstance().create(1, testProblemId_, "/tmp/test2.cpp");
    auto sub = SubmissionModel::getInstance().findById(id);
    ASSERT_TRUE(sub.has_value());
    EXPECT_EQ(sub->status, "pending");
    EXPECT_EQ(sub->userId, 1);
}

TEST_F(SubmissionModelTest, FindByIdNotFound) {
    auto sub = SubmissionModel::getInstance().findById(99999999);
    EXPECT_FALSE(sub.has_value());
}

TEST_F(SubmissionModelTest, UpdateResult) {
    int id = SubmissionModel::getInstance().create(1, 1, "/tmp/test3.cpp");
    SubmissionModel::getInstance().updateResult(id, "accepted", 42, 1024, "");

    auto sub = SubmissionModel::getInstance().findById(id);
    ASSERT_TRUE(sub.has_value());
    EXPECT_EQ(sub->status, "accepted");
    EXPECT_EQ(sub->timeUsedMs, 42);
    EXPECT_EQ(sub->memoryUsedKb, 1024);
}

TEST_F(SubmissionModelTest, SaveAndGetDetails) {
    int id = SubmissionModel::getInstance().create(1, testProblemId_, "/tmp/test4.cpp");
    SubmissionModel::getInstance().saveDetail(id, testCaseId_, "accepted", 10, 256, "42");

    auto details = SubmissionModel::getInstance().getDetails(id);
    EXPECT_EQ(details.size(), 1);
    EXPECT_EQ(details[0].status, "accepted");
    EXPECT_EQ(details[0].actualOutput, "42");
}

TEST_F(SubmissionModelTest, ListByUser) {
    SubmissionModel::getInstance().create(1, 1, "/tmp/a.cpp");
    SubmissionModel::getInstance().create(1, 2, "/tmp/b.cpp");

    auto subs = SubmissionModel::getInstance().listByUser(1, 1, 10);
    EXPECT_GE(subs.size(), 2);
}

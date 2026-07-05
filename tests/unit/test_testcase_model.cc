#include <gtest/gtest.h>
#include "models/problem.h"
#include "models/testcase.h"
#include "db/connection_pool.h"

class TestCaseModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!ConnectionPool::getInstance().total()) {
            ConnectionPool::getInstance().init(
                "127.0.0.1", 3306, "root", "2791830200Zby..",
                "oj_platform", 3);
        }
        Problem p;
        p.title = "TC Test Problem " + std::to_string(time(nullptr));
        p.description = "For testing";
        p.difficulty = "easy";
        p.timeLimitMs = 1000;
        p.memoryLimitKb = 262144;
        p.createdBy = 1;
        problemId_ = ProblemModel::getInstance().create(p);
    }

    void TearDown() override {
        TestCaseModel::getInstance().removeByProblemId(problemId_);
        ProblemModel::getInstance().remove(problemId_);
    }

    int problemId_;
};

TEST_F(TestCaseModelTest, AddTestCase) {
    int id = TestCaseModel::getInstance().add(problemId_, "1 2", "3", true, 0);
    EXPECT_GT(id, 0);
}

TEST_F(TestCaseModelTest, FindByProblemIdSamplesOnly) {
    TestCaseModel::getInstance().add(problemId_, "1 2", "3", true, 0);
    TestCaseModel::getInstance().add(problemId_, "4 5", "9", false, 1);

    auto samples = TestCaseModel::getInstance().findByProblemId(problemId_, true);
    ASSERT_EQ(samples.size(), 1);
    EXPECT_EQ(samples[0].input, "1 2");
    EXPECT_EQ(samples[0].expectedOutput, "3");
    EXPECT_TRUE(samples[0].isSample);
}

TEST_F(TestCaseModelTest, FindAllByProblemId) {
    TestCaseModel::getInstance().add(problemId_, "a", "A", true, 0);
    TestCaseModel::getInstance().add(problemId_, "b", "B", false, 1);
    TestCaseModel::getInstance().add(problemId_, "c", "C", false, 2);

    auto all = TestCaseModel::getInstance().findAllByProblemId(problemId_);
    EXPECT_EQ(all.size(), 3);
}

TEST_F(TestCaseModelTest, FindById) {
    int id = TestCaseModel::getInstance().add(problemId_, "x", "y", true, 0);
    auto tc = TestCaseModel::getInstance().findById(id);
    ASSERT_TRUE(tc.has_value());
    EXPECT_EQ(tc->input, "x");
    EXPECT_EQ(tc->expectedOutput, "y");
}

TEST_F(TestCaseModelTest, DeleteTestCase) {
    int id = TestCaseModel::getInstance().add(problemId_, "del", "DEL", true, 0);
    bool ok = TestCaseModel::getInstance().remove(id);
    EXPECT_TRUE(ok);

    auto found = TestCaseModel::getInstance().findById(id);
    EXPECT_FALSE(found.has_value());
}

TEST_F(TestCaseModelTest, OrderIndex) {
    TestCaseModel::getInstance().add(problemId_, "first", "1", false, 0);
    TestCaseModel::getInstance().add(problemId_, "second", "2", false, 1);

    auto all = TestCaseModel::getInstance().findAllByProblemId(problemId_);
    ASSERT_GE(all.size(), 2);
    EXPECT_EQ(all[0].orderIndex, 0);
    EXPECT_EQ(all[1].orderIndex, 1);
}

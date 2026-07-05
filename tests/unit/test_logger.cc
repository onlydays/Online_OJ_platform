#include <gtest/gtest.h>
#include "utils/logger.h"
#include <sstream>
#include <iostream>

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::getInstance().setLevel(LogLevel::DEBUG);
        oldCout_ = std::cout.rdbuf();
        oldCerr_ = std::cerr.rdbuf();
    }

    void TearDown() override {
        std::cout.rdbuf(oldCout_);
        std::cerr.rdbuf(oldCerr_);
        Logger::getInstance().setLevel(LogLevel::DEBUG);
    }

    std::string captureCout() {
        std::cout.rdbuf(bufferCout_.rdbuf());
        std::cout.clear();
        return "";
    }

    std::string captureCerr() {
        std::cerr.rdbuf(bufferCerr_.rdbuf());
        std::cerr.clear();
        return "";
    }

    std::string getCout() { return bufferCout_.str(); }
    std::string getCerr() { return bufferCerr_.str(); }

    std::stringstream bufferCout_;
    std::stringstream bufferCerr_;
    std::streambuf* oldCout_;
    std::streambuf* oldCerr_;
};

// ========== 单例 ==========

TEST_F(LoggerTest, Singleton) {
    Logger& a = Logger::getInstance();
    Logger& b = Logger::getInstance();
    EXPECT_EQ(&a, &b);
}

// ========== 各日志级别输出 ==========

TEST_F(LoggerTest, DebugOutput) {
    captureCout();
    LOG_DEBUG("test message");
    std::string output = getCout();
    EXPECT_NE(output.find("DEBUG"), std::string::npos);
    EXPECT_NE(output.find("test message"), std::string::npos);
}

TEST_F(LoggerTest, InfoOutput) {
    captureCout();
    LOG_INFO("info message");
    std::string output = getCout();
    EXPECT_NE(output.find("INFO"), std::string::npos);
    EXPECT_NE(output.find("info message"), std::string::npos);
}

TEST_F(LoggerTest, WarnOutput) {
    captureCerr();
    LOG_WARN("warn message");
    std::string output = getCerr();
    EXPECT_NE(output.find("WARN"), std::string::npos);
    EXPECT_NE(output.find("warn message"), std::string::npos);
}

TEST_F(LoggerTest, ErrorOutput) {
    captureCerr();
    LOG_ERROR("error message");
    std::string output = getCerr();
    EXPECT_NE(output.find("ERROR"), std::string::npos);
    EXPECT_NE(output.find("error message"), std::string::npos);
}

// ========== 日志级别过滤 ==========

TEST_F(LoggerTest, LevelFilterBlocksDebug) {
    Logger::getInstance().setLevel(LogLevel::INFO);
    captureCout();
    LOG_DEBUG("should not appear");
    std::string output = getCout();
    EXPECT_EQ(output.find("should not appear"), std::string::npos);
}

TEST_F(LoggerTest, LevelFilterPassesError) {
    Logger::getInstance().setLevel(LogLevel::ERROR);
    captureCerr();
    LOG_ERROR("critical");
    std::string output = getCerr();
    EXPECT_NE(output.find("critical"), std::string::npos);
}

TEST_F(LoggerTest, LevelFilterInfoBlockedAtWarn) {
    Logger::getInstance().setLevel(LogLevel::WARN);
    captureCout();
    LOG_INFO("should not log");
    std::string output = getCout();
    EXPECT_EQ(output.find("should not log"), std::string::npos);
}

// ========== 时间戳 ==========

TEST_F(LoggerTest, TimestampPresent) {
    captureCout();
    LOG_INFO("timestamp test");
    std::string output = getCout();
    EXPECT_NE(output.find("202"), std::string::npos);
    EXPECT_NE(output.find("-"), std::string::npos);
    EXPECT_NE(output.find(":"), std::string::npos);
}

// ========== 文件行号 ==========

TEST_F(LoggerTest, FileAndLinePresent) {
    captureCout();
    LOG_INFO("check location");
    std::string output = getCout();
    EXPECT_NE(output.find("test_logger.cc"), std::string::npos);
    EXPECT_NE(output.find(":"), std::string::npos);
}

// ========== 流式输出 ==========

TEST_F(LoggerTest, StreamStyleOutput) {
    captureCout();
    LOG_INFO("answer is " << 42 << " and " << 3.14);
    std::string output = getCout();
    EXPECT_NE(output.find("answer is 42 and 3.14"), std::string::npos);
}

#include <gtest/gtest.h>
#include "utils/config.h"
#include <fstream>
#include <stdexcept>

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        tempFile_ = "/tmp/test_config_unit.yaml";
        std::ofstream out(tempFile_);
        out << "server:\n";
        out << "  host: \"0.0.0.0\"\n";
        out << "  port: 8080\n";
        out << "\n";
        out << "database:\n";
        out << "  host: \"localhost\"\n";
        out << "  port: 3306\n";
        out << "  username: \"testuser\"\n";
        out << "  password: \"secret\"\n";
        out << "  max_connections: 10\n";
        out << "\n";
        out << "# 下面是空值键\n";
        out << "empty_key:\n";
        out << "nested:\n";
        out << "  level1:\n";
        out << "    level2: \"deep_value\"\n";
        out.close();
    }

    void TearDown() override {
        std::remove(tempFile_.c_str());
    }

    std::string tempFile_;
};

// ========== 单例测试 ==========

TEST_F(ConfigManagerTest, SingletonReturnsSameInstance) {
    ConfigManager& a = ConfigManager::getInstance();
    ConfigManager& b = ConfigManager::getInstance();
    EXPECT_EQ(&a, &b);
}

// ========== 基本加载测试 ==========

TEST_F(ConfigManagerTest, LoadYAMLFile) {
    auto& cfg = ConfigManager::getInstance();
    EXPECT_NO_THROW(cfg.loadFromYAML(tempFile_));
}

TEST_F(ConfigManagerTest, LoadNonExistentFile) {
    auto& cfg = ConfigManager::getInstance();
    EXPECT_THROW(cfg.loadFromYAML("/tmp/nonexistent_xyz.yaml"), std::runtime_error);
}

// ========== 字符串值解析 ==========

TEST_F(ConfigManagerTest, GetStringBasic) {
    auto& cfg = ConfigManager::getInstance();
    cfg.loadFromYAML(tempFile_);
    EXPECT_EQ(cfg.getString("server.host"), "0.0.0.0");
    EXPECT_EQ(cfg.getString("database.host"), "localhost");
    EXPECT_EQ(cfg.getString("database.username"), "testuser");
    EXPECT_EQ(cfg.getString("database.password"), "secret");
}

// ========== 整数解析 ==========

TEST_F(ConfigManagerTest, GetIntBasic) {
    auto& cfg = ConfigManager::getInstance();
    cfg.loadFromYAML(tempFile_);
    EXPECT_EQ(cfg.getInt("server.port"), 8080);
    EXPECT_EQ(cfg.getInt("database.port"), 3306);
    EXPECT_EQ(cfg.getInt("database.max_connections"), 10);
}

// ========== 默认值 ==========

TEST_F(ConfigManagerTest, GetStringDefault) {
    auto& cfg = ConfigManager::getInstance();
    cfg.loadFromYAML(tempFile_);
    EXPECT_EQ(cfg.getString("nonexistent.key"), "");
    EXPECT_EQ(cfg.getString("nonexistent.key", "fallback"), "fallback");
}

TEST_F(ConfigManagerTest, GetIntDefault) {
    auto& cfg = ConfigManager::getInstance();
    cfg.loadFromYAML(tempFile_);
    EXPECT_EQ(cfg.getInt("nonexistent.key"), 0);
    EXPECT_EQ(cfg.getInt("nonexistent.key", 42), 42);
}

// ========== 空值处理 ==========

TEST_F(ConfigManagerTest, EmptyValueKey) {
    auto& cfg = ConfigManager::getInstance();
    cfg.loadFromYAML(tempFile_);
    EXPECT_EQ(cfg.getString("empty_key"), "");
}

// ========== 深层嵌套 ==========

TEST_F(ConfigManagerTest, DeepNesting) {
    auto& cfg = ConfigManager::getInstance();
    cfg.loadFromYAML(tempFile_);
    EXPECT_EQ(cfg.getString("nested.level1.level2"), "deep_value");
}

// ========== 注释和空行 ==========

TEST_F(ConfigManagerTest, CommentsAndBlankLinesIgnored) {
    auto& cfg = ConfigManager::getInstance();
    cfg.loadFromYAML(tempFile_);
    EXPECT_FALSE(cfg.getString("#").empty() == false && cfg.getString("#") == "#");
    EXPECT_EQ(cfg.getString("server.host"), "0.0.0.0");
}

// ========== 引号剥离 ==========

TEST_F(ConfigManagerTest, QuotedStringsStripped) {
    auto& cfg = ConfigManager::getInstance();
    cfg.loadFromYAML(tempFile_);
    EXPECT_EQ(cfg.getString("server.host"), "0.0.0.0");
    EXPECT_EQ(cfg.getString("database.username"), "testuser");
}

// ========== 整数边界 ==========

TEST_F(ConfigManagerTest, GetIntInvalidFormat) {
    auto& cfg = ConfigManager::getInstance();
    cfg.loadFromYAML(tempFile_);
    EXPECT_EQ(cfg.getInt("server.host", -1), -1);
}

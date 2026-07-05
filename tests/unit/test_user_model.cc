#include <gtest/gtest.h>
#include "models/user.h"
#include "db/connection_pool.h"
#include "auth.h"
#include <cppconn/exception.h>

class UserModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!ConnectionPool::getInstance().total()) {
            ConnectionPool::getInstance().init(
                "127.0.0.1", 3306, "root", "2791830200Zby..",
                "oj_platform", 3);
        }
    }
};

// ========== 创建用户 ==========

TEST_F(UserModelTest, CreateUser) {
    std::string testUser = "testuser_" + std::to_string(time(nullptr));
    int id = UserModel::getInstance().create(testUser, "hash123", "user");
    EXPECT_GT(id, 0);
}

TEST_F(UserModelTest, CreateDuplicateUser) {
    std::string testUser = "dupuser_" + std::to_string(time(nullptr));
    UserModel::getInstance().create(testUser, "hash1", "user");

    EXPECT_THROW(
        UserModel::getInstance().create(testUser, "hash2", "user"),
        sql::SQLException
    );
}

// ========== 查找用户 ==========

TEST_F(UserModelTest, FindById) {
    std::string testUser = "findbyid_" + std::to_string(time(nullptr));
    int id = UserModel::getInstance().create(testUser, "hash", "user");

    auto user = UserModel::getInstance().findById(id);
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->username, testUser);
    EXPECT_EQ(user->role, "user");
}

TEST_F(UserModelTest, FindByIdNotFound) {
    auto user = UserModel::getInstance().findById(99999999);
    EXPECT_FALSE(user.has_value());
}

TEST_F(UserModelTest, FindByUsername) {
    std::string testUser = "findbyname_" + std::to_string(time(nullptr));
    UserModel::getInstance().create(testUser, "hash", "admin");

    auto user = UserModel::getInstance().findByUsername(testUser);
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->role, "admin");
}

TEST_F(UserModelTest, FindByUsernameNotFound) {
    auto user = UserModel::getInstance().findByUsername("nonexistent_user_xyz");
    EXPECT_FALSE(user.has_value());
}

// ========== 密码验证 ==========

TEST_F(UserModelTest, PasswordHashMatch) {
    std::string testUser = "pwuser_" + std::to_string(time(nullptr));
    std::string password = "mypassword";
    std::string hash = auth::sha256(password);
    UserModel::getInstance().create(testUser, hash, "user");

    auto user = UserModel::getInstance().findByUsername(testUser);
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->passwordHash, hash);
    EXPECT_EQ(user->passwordHash, auth::sha256(password));
}

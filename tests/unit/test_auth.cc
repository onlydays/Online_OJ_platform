#include <gtest/gtest.h>
#include "auth.h"
#include "utils/config.h"

// ========== SHA-256 (无需配置) ==========

TEST(AuthSha256Test, Deterministic) {
    std::string h1 = auth::sha256("hello");
    std::string h2 = auth::sha256("hello");
    EXPECT_EQ(h1, h2);
}

TEST(AuthSha256Test, DifferentInputs) {
    std::string h1 = auth::sha256("hello");
    std::string h2 = auth::sha256("world");
    EXPECT_NE(h1, h2);
}

TEST(AuthSha256Test, Length) {
    std::string hash = auth::sha256("test");
    EXPECT_EQ(hash.size(), 64);
}

TEST(AuthSha256Test, HexFormat) {
    std::string hash = auth::sha256("abc");
    for (char c : hash) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    }
}

TEST(AuthSha256Test, KnownValue) {
    std::string hash = auth::sha256("admin123");
    EXPECT_EQ(hash, "240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9");
}

// ========== JWT (需要配置) ==========

class JwtTest : public ::testing::Test {
protected:
    void SetUp() override {
        ConfigManager::getInstance().loadFromYAML("../config.yaml");
    }
};

TEST_F(JwtTest, GenerateToken) {
    std::string token = auth::generateToken(1, "testuser", "user");
    EXPECT_GT(token.size(), 10);
    EXPECT_NE(token.find('.'), std::string::npos);
}

TEST_F(JwtTest, VerifyValidToken) {
    std::string token = auth::generateToken(42, "alice", "admin");
    auto userId = auth::verifyToken(token);
    ASSERT_TRUE(userId.has_value());
    EXPECT_EQ(*userId, 42);
}

TEST_F(JwtTest, VerifyInvalidToken) {
    auto userId = auth::verifyToken("invalid.token.here");
    EXPECT_FALSE(userId.has_value());
}

TEST_F(JwtTest, VerifyEmptyToken) {
    auto userId = auth::verifyToken("");
    EXPECT_FALSE(userId.has_value());
}

TEST_F(JwtTest, ExtractUsername) {
    std::string token = auth::generateToken(1, "bob", "user");
    EXPECT_EQ(auth::extractUsername(token), "bob");
}

TEST_F(JwtTest, ExtractRole) {
    std::string token = auth::generateToken(1, "charlie", "admin");
    EXPECT_EQ(auth::extractRole(token), "admin");
}

TEST_F(JwtTest, DifferentTokensForDifferentUsers) {
    std::string t1 = auth::generateToken(1, "u1", "user");
    std::string t2 = auth::generateToken(2, "u2", "user");
    EXPECT_NE(t1, t2);
}

#include <gtest/gtest.h>
#include "db/connection_pool.h"
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <thread>
#include <vector>
#include <atomic>

class ConnectionPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        pool_ = &ConnectionPool::getInstance();
        if (!pool_->total()) {
            pool_->init("127.0.0.1", 3306, "root", "2791830200Zby..",
                        "oj_platform", 5);
        }
    }

    ConnectionPool* pool_ = nullptr;
};

// ========== 单例 ==========

TEST_F(ConnectionPoolTest, Singleton) {
    ConnectionPool& a = ConnectionPool::getInstance();
    ConnectionPool& b = ConnectionPool::getInstance();
    EXPECT_EQ(&a, &b);
}

// ========== 初始状态 ==========

TEST_F(ConnectionPoolTest, InitialState) {
    EXPECT_EQ(pool_->total(), 5);
    EXPECT_EQ(pool_->available(), 5);
}

// ========== 获取和归还连接 ==========

TEST_F(ConnectionPoolTest, GetAndReleaseConnection) {
    EXPECT_EQ(pool_->available(), 5);
    {
        auto conn = pool_->getConnection();
        EXPECT_TRUE(conn != nullptr);
        EXPECT_EQ(pool_->available(), 4);
    }
    EXPECT_EQ(pool_->available(), 5);
}

// ========== 连接验证 (执行查询) ==========

TEST_F(ConnectionPoolTest, ConnectionCanExecuteQuery) {
    auto conn = pool_->getConnection();
    ASSERT_TRUE(conn != nullptr);

    std::unique_ptr<sql::Statement> stmt(conn->createStatement());
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery("SELECT 1 AS val"));
    ASSERT_TRUE(rs->next());
    EXPECT_EQ(rs->getInt("val"), 1);
}

// ========== 连接验证 (访问 oj_platform 表) ==========

TEST_F(ConnectionPoolTest, CanAccessTables) {
    auto conn = pool_->getConnection();
    ASSERT_TRUE(conn != nullptr);

    std::unique_ptr<sql::Statement> stmt(conn->createStatement());
    std::unique_ptr<sql::ResultSet> rs(
        stmt->executeQuery("SELECT COUNT(*) AS cnt FROM users"));
    ASSERT_TRUE(rs->next());
    EXPECT_GE(rs->getInt("cnt"), 0);
}

// ========== 多线程并发获取连接 ==========

TEST_F(ConnectionPoolTest, ConcurrentConnections) {
    const int N = 5;
    std::atomic<int> success{0};
    std::vector<std::thread> threads;

    for (int i = 0; i < N; ++i) {
        threads.emplace_back([this, &success]() {
            auto conn = pool_->getConnection();
            if (conn) ++success;
        });
    }
    for (auto& t : threads) t.join();

    EXPECT_EQ(success.load(), N);
}

// ========== 销毁重建 ==========

TEST_F(ConnectionPoolTest, DestroyAndReinit) {
    EXPECT_EQ(pool_->available(), 5);
    pool_->destroy();
    EXPECT_EQ(pool_->available(), 0);

    pool_->init("127.0.0.1", 3306, "root", "2791830200Zby..",
                "oj_platform", 3);
    EXPECT_EQ(pool_->total(), 3);
    EXPECT_EQ(pool_->available(), 3);
}

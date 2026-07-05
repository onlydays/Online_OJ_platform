#include "user.h"
#include "db/connection_pool.h"
#include "utils/logger.h"

#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

UserModel& UserModel::getInstance() {
    static UserModel instance;
    return instance;
}

std::optional<User> UserModel::findById(int id) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return std::nullopt;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement("SELECT id, username, password_hash, role, created_at "
                               "FROM users WHERE id = ?"));
    stmt->setInt(1, id);

    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) return std::nullopt;

    User u;
    u.id = rs->getInt("id");
    u.username = rs->getString("username");
    u.passwordHash = rs->getString("password_hash");
    u.role = rs->getString("role");
    u.createdAt = rs->getString("created_at");
    return u;
}

std::optional<User> UserModel::findByUsername(const std::string& username) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return std::nullopt;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement("SELECT id, username, password_hash, role, created_at "
                               "FROM users WHERE username = ?"));
    stmt->setString(1, username);

    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) return std::nullopt;

    User u;
    u.id = rs->getInt("id");
    u.username = rs->getString("username");
    u.passwordHash = rs->getString("password_hash");
    u.role = rs->getString("role");
    u.createdAt = rs->getString("created_at");
    return u;
}

int UserModel::create(const std::string& username, const std::string& passwordHash,
                      const std::string& role) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return -1;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement("INSERT INTO users (username, password_hash, role) "
                               "VALUES (?, ?, ?)"));
    stmt->setString(1, username);
    stmt->setString(2, passwordHash);
    stmt->setString(3, role);
    stmt->executeUpdate();

    std::unique_ptr<sql::Statement> idStmt(conn->createStatement());
    std::unique_ptr<sql::ResultSet> rs(
        idStmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
    rs->next();
    return rs->getInt("id");
}

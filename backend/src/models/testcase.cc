#include "testcase.h"
#include "db/connection_pool.h"

#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

TestCaseModel& TestCaseModel::getInstance() {
    static TestCaseModel instance;
    return instance;
}

static TestCase rowToTestCase(sql::ResultSet* rs) {
    TestCase tc;
    tc.id = rs->getInt("id");
    tc.problemId = rs->getInt("problem_id");
    tc.input = rs->getString("input");
    tc.expectedOutput = rs->getString("expected_output");
    tc.isSample = rs->getBoolean("is_sample");
    tc.orderIndex = rs->getInt("order_index");
    tc.createdAt = rs->getString("created_at");
    return tc;
}

std::vector<TestCase> TestCaseModel::findByProblemId(int problemId, bool samplesOnly) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return {};

    std::string sql = "SELECT * FROM test_cases WHERE problem_id = ?";
    if (samplesOnly) sql += " AND is_sample = TRUE";
    sql += " ORDER BY order_index ASC";

    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(sql));
    stmt->setInt(1, problemId);

    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<TestCase> result;
    while (rs->next()) {
        result.push_back(rowToTestCase(rs.get()));
    }
    return result;
}

std::vector<TestCase> TestCaseModel::findAllByProblemId(int problemId) {
    return findByProblemId(problemId, false);
}

std::optional<TestCase> TestCaseModel::findById(int id) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return std::nullopt;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement("SELECT * FROM test_cases WHERE id = ?"));
    stmt->setInt(1, id);

    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) return std::nullopt;
    return rowToTestCase(rs.get());
}

int TestCaseModel::add(int problemId, const std::string& input,
                       const std::string& expectedOutput, bool isSample,
                       int orderIndex) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return -1;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "INSERT INTO test_cases (problem_id, input, expected_output, "
            "is_sample, order_index) VALUES (?, ?, ?, ?, ?)"));
    stmt->setInt(1, problemId);
    stmt->setString(2, input);
    stmt->setString(3, expectedOutput);
    stmt->setBoolean(4, isSample);
    stmt->setInt(5, orderIndex);
    stmt->executeUpdate();

    std::unique_ptr<sql::Statement> idStmt(conn->createStatement());
    std::unique_ptr<sql::ResultSet> rs(
        idStmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
    rs->next();
    return rs->getInt("id");
}

bool TestCaseModel::remove(int id) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return false;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement("DELETE FROM test_cases WHERE id=?"));
    stmt->setInt(1, id);
    return stmt->executeUpdate() > 0;
}

bool TestCaseModel::removeByProblemId(int problemId) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return false;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement("DELETE FROM test_cases WHERE problem_id=?"));
    stmt->setInt(1, problemId);
    stmt->executeUpdate();
    return true;
}

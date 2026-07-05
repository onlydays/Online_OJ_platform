#include "submission.h"
#include "db/connection_pool.h"

#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <sstream>

SubmissionModel& SubmissionModel::getInstance() {
    static SubmissionModel instance;
    return instance;
}

int SubmissionModel::create(int userId, int problemId, const std::string& codePath) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return -1;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "INSERT INTO submissions (user_id, problem_id, code_path, status) "
            "VALUES (?, ?, ?, 'pending')"));
    stmt->setInt(1, userId);
    stmt->setInt(2, problemId);
    stmt->setString(3, codePath);
    stmt->executeUpdate();

    std::unique_ptr<sql::Statement> idStmt(conn->createStatement());
    std::unique_ptr<sql::ResultSet> rs(
        idStmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
    rs->next();
    return rs->getInt("id");
}

std::optional<Submission> SubmissionModel::findById(int id) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return std::nullopt;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement("SELECT * FROM submissions WHERE id = ?"));
    stmt->setInt(1, id);

    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) return std::nullopt;

    Submission s;
    s.id = rs->getInt("id");
    s.userId = rs->getInt("user_id");
    s.problemId = rs->getInt("problem_id");
    s.codePath = rs->getString("code_path");
    s.status = rs->getString("status");
    s.timeUsedMs = rs->getInt("time_used_ms");
    s.memoryUsedKb = rs->getInt("memory_used_kb");
    s.compileError = rs->getString("compile_error");
    s.createdAt = rs->getString("created_at");
    return s;
}

std::vector<Submission> SubmissionModel::listByUser(int userId, int page, int size) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return {};

    std::ostringstream sql;
    sql << "SELECT * FROM submissions WHERE user_id = " << userId
        << " ORDER BY id DESC LIMIT " << size
        << " OFFSET " << ((page - 1) * size);

    std::unique_ptr<sql::Statement> stmt(conn->createStatement());
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery(sql.str()));

    std::vector<Submission> result;
    while (rs->next()) {
        Submission s;
        s.id = rs->getInt("id");
        s.userId = rs->getInt("user_id");
        s.problemId = rs->getInt("problem_id");
        s.codePath = rs->getString("code_path");
        s.status = rs->getString("status");
        s.timeUsedMs = rs->getInt("time_used_ms");
        s.memoryUsedKb = rs->getInt("memory_used_kb");
        s.compileError = rs->getString("compile_error");
        s.createdAt = rs->getString("created_at");
        result.push_back(s);
    }
    return result;
}

void SubmissionModel::updateResult(int id, const std::string& status,
                                   int timeMs, int memoryKb,
                                   const std::string& compileError) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "UPDATE submissions SET status=?, time_used_ms=?, memory_used_kb=?, "
            "compile_error=? WHERE id=?"));
    stmt->setString(1, status);
    stmt->setInt(2, timeMs);
    stmt->setInt(3, memoryKb);
    stmt->setString(4, compileError);
    stmt->setInt(5, id);
    stmt->executeUpdate();
}

void SubmissionModel::saveDetail(int submissionId, int testCaseId,
                                 const std::string& status, int timeMs,
                                 int memoryKb, const std::string& actualOutput) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "INSERT INTO submission_results "
            "(submission_id, test_case_id, status, time_used_ms, memory_used_kb, actual_output) "
            "VALUES (?, ?, ?, ?, ?, ?)"));
    stmt->setInt(1, submissionId);
    stmt->setInt(2, testCaseId);
    stmt->setString(3, status);
    stmt->setInt(4, timeMs);
    stmt->setInt(5, memoryKb);
    stmt->setString(6, actualOutput);
    stmt->executeUpdate();
}

std::vector<SubmissionDetail> SubmissionModel::getDetails(int submissionId) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return {};

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "SELECT status, time_used_ms, memory_used_kb, actual_output "
            "FROM submission_results WHERE submission_id = ? ORDER BY id"));
    stmt->setInt(1, submissionId);

    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<SubmissionDetail> result;
    while (rs->next()) {
        SubmissionDetail d;
        d.status = rs->getString("status");
        d.timeUsedMs = rs->getInt("time_used_ms");
        d.memoryUsedKb = rs->getInt("memory_used_kb");
        d.actualOutput = rs->getString("actual_output");
        result.push_back(d);
    }
    return result;
}

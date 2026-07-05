#include "problem.h"
#include "db/connection_pool.h"

#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <sstream>

ProblemModel& ProblemModel::getInstance() {
    static ProblemModel instance;
    return instance;
}

static Problem rowToProblem(sql::ResultSet* rs) {
    Problem p;
    p.id = rs->getInt("id");
    p.title = rs->getString("title");
    p.description = rs->getString("description");
    p.difficulty = rs->getString("difficulty");
    p.timeLimitMs = rs->getInt("time_limit_ms");
    p.memoryLimitKb = rs->getInt("memory_limit_kb");
    p.createdBy = rs->getInt("created_by");
    p.createdAt = rs->getString("created_at");
    p.updatedAt = rs->getString("updated_at");
    return p;
}

std::optional<Problem> ProblemModel::findById(int id) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return std::nullopt;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "SELECT p.*, "
            "COALESCE((SELECT COUNT(*) FROM submissions s "
            "  WHERE s.problem_id = p.id AND s.status = 'accepted') * 100.0 / "
            "NULLIF((SELECT COUNT(*) FROM submissions s WHERE s.problem_id = p.id), 0), 0) AS ac_rate "
            "FROM problems p WHERE p.id = ?"));
    stmt->setInt(1, id);

    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) return std::nullopt;

    Problem p = rowToProblem(rs.get());
    p.acRate = rs->getDouble("ac_rate");
    return p;
}

std::vector<ProblemListItem> ProblemModel::list(const std::string& difficulty,
                                                 int page, int size) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return {};

    std::ostringstream sql;
    sql << "SELECT p.id, p.title, p.difficulty, "
        << "COALESCE((SELECT COUNT(*) FROM submissions s "
        << "  WHERE s.problem_id = p.id AND s.status = 'accepted') * 100.0 / "
        << "NULLIF((SELECT COUNT(*) FROM submissions s WHERE s.problem_id = p.id), 0), 0) AS ac_rate "
        << "FROM problems p ";

    if (!difficulty.empty()) {
        sql << "WHERE p.difficulty = '" << difficulty << "' ";
    }
    sql << "ORDER BY p.id ASC LIMIT " << size << " OFFSET " << ((page - 1) * size);

    std::unique_ptr<sql::Statement> stmt(conn->createStatement());
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery(sql.str()));

    std::vector<ProblemListItem> result;
    while (rs->next()) {
        result.push_back({
            rs->getInt("id"),
            rs->getString("title"),
            rs->getString("difficulty"),
            static_cast<double>(rs->getDouble("ac_rate"))
        });
    }
    return result;
}

int ProblemModel::total(const std::string& difficulty) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return 0;

    std::ostringstream sql;
    sql << "SELECT COUNT(*) AS cnt FROM problems";
    if (!difficulty.empty()) {
        sql << " WHERE difficulty = '" << difficulty << "'";
    }

    std::unique_ptr<sql::Statement> stmt(conn->createStatement());
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery(sql.str()));
    rs->next();
    return rs->getInt("cnt");
}

int ProblemModel::create(const Problem& p) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return -1;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "INSERT INTO problems (title, description, difficulty, "
            "time_limit_ms, memory_limit_kb, created_by) "
            "VALUES (?, ?, ?, ?, ?, ?)"));
    stmt->setString(1, p.title);
    stmt->setString(2, p.description);
    stmt->setString(3, p.difficulty);
    stmt->setInt(4, p.timeLimitMs);
    stmt->setInt(5, p.memoryLimitKb);
    stmt->setInt(6, p.createdBy);
    stmt->executeUpdate();

    std::unique_ptr<sql::Statement> idStmt(conn->createStatement());
    std::unique_ptr<sql::ResultSet> rs(
        idStmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
    rs->next();
    return rs->getInt("id");
}

bool ProblemModel::update(int id, const Problem& p) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return false;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "UPDATE problems SET title=?, description=?, difficulty=?, "
            "time_limit_ms=?, memory_limit_kb=? WHERE id=?"));
    stmt->setString(1, p.title);
    stmt->setString(2, p.description);
    stmt->setString(3, p.difficulty);
    stmt->setInt(4, p.timeLimitMs);
    stmt->setInt(5, p.memoryLimitKb);
    stmt->setInt(6, id);
    return stmt->executeUpdate() > 0;
}

bool ProblemModel::remove(int id) {
    auto conn = ConnectionPool::getInstance().getConnection();
    if (!conn) return false;

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement("DELETE FROM problems WHERE id=?"));
    stmt->setInt(1, id);
    return stmt->executeUpdate() > 0;
}

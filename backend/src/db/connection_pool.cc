#include "connection_pool.h"
#include "utils/logger.h"

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>

ConnectionPool& ConnectionPool::getInstance() {
    static ConnectionPool instance;
    return instance;
}

ConnectionPool::ConnectionPool()
    : host_(""), user_(""), password_(""), database_(""),
      port_(3306), poolSize_(10), initialized_(false) {}

ConnectionPool::~ConnectionPool() {
    destroy();
}

void ConnectionPool::init(const std::string& host, int port,
                          const std::string& user, const std::string& password,
                          const std::string& database, int poolSize) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_) return;

    host_ = host;
    port_ = port;
    user_ = user;
    password_ = password;
    database_ = database;
    poolSize_ = poolSize;

    for (int i = 0; i < poolSize_; ++i) {
        sql::Connection* conn = createConnection();
        if (conn) {
            pool_.push(conn);
        }
    }

    initialized_ = true;
    LOG_INFO("Connection pool initialized: " << pool_.size()
             << "/" << poolSize_ << " connections");
}

sql::Connection* ConnectionPool::createConnection() {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::string url = "tcp://" + host_ + ":" + std::to_string(port_);
        sql::Connection* conn = driver->connect(url, user_, password_);
        conn->setSchema(database_);
        return conn;
    } catch (sql::SQLException& e) {
        LOG_ERROR("Failed to create MySQL connection: " << e.what());
        return nullptr;
    }
}

ConnectionPool::ConnPtr ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return !pool_.empty(); });

    sql::Connection* raw = pool_.front();
    pool_.pop();

    try {
        std::unique_ptr<sql::Statement> stmt(raw->createStatement());
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery("SELECT 1"));
        rs->next();
    } catch (sql::SQLException&) {
        delete raw;
        raw = createConnection();
        if (!raw) {
            pool_.push(nullptr);
            return ConnPtr(nullptr);
        }
    }

    return ConnPtr(raw, [this](sql::Connection* conn) {
        releaseConnection(conn);
    });
}

void ConnectionPool::releaseConnection(sql::Connection* conn) {
    if (!conn) return;
    std::lock_guard<std::mutex> lock(mutex_);
    pool_.push(conn);
    cond_.notify_one();
}

void ConnectionPool::destroy() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!pool_.empty()) {
        delete pool_.front();
        pool_.pop();
    }
    initialized_ = false;
}

int ConnectionPool::available() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int>(pool_.size());
}

int ConnectionPool::total() const {
    if (!initialized_) return 0;
    return poolSize_;
}

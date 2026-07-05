#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <memory>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace sql {
    class Connection;
}

class ConnectionPool {
public:
    using ConnPtr = std::shared_ptr<sql::Connection>;

    static ConnectionPool& getInstance();

    void init(const std::string& host, int port,
              const std::string& user, const std::string& password,
              const std::string& database, int poolSize = 10);

    ConnPtr getConnection();
    void destroy();

    int available() const;
    int total() const;

private:
    ConnectionPool();
    ~ConnectionPool();
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    sql::Connection* createConnection();
    void releaseConnection(sql::Connection* conn);

    std::string host_, user_, password_, database_;
    int port_;
    int poolSize_;
    bool initialized_;

    std::queue<sql::Connection*> pool_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;
};

#endif // CONNECTION_POOL_H

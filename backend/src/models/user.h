#ifndef USER_MODEL_H
#define USER_MODEL_H

#include <string>
#include <optional>

struct User {
    int id;
    std::string username;
    std::string passwordHash;
    std::string role;
    std::string createdAt;
};

class UserModel {
public:
    static UserModel& getInstance();

    std::optional<User> findById(int id);
    std::optional<User> findByUsername(const std::string& username);
    int create(const std::string& username, const std::string& passwordHash,
               const std::string& role = "user");

private:
    UserModel() = default;
};

#endif // USER_MODEL_H

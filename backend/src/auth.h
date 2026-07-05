#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <optional>

namespace auth {

std::string sha256(const std::string& input);

std::string generateToken(int userId, const std::string& username, const std::string& role);
std::optional<int> verifyToken(const std::string& token);
std::string  extractUsername(const std::string& token);
std::string  extractRole(const std::string& token);

} // namespace auth

#endif // AUTH_H

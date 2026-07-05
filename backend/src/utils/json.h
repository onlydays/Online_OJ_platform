#ifndef OJ_JSON_H
#define OJ_JSON_H

#include "json.hpp"

using Json = nlohmann::json;

namespace json_utils {

std::string serialize(const Json& j);
Json        deserialize(const std::string& str);

Json errorResponse(int code, const std::string& message);
Json successResponse(const Json& data);
Json successResponse(const std::string& key, const Json& value);

Json arrayResponse(const std::string& key, const Json& arr);

Json getOrDefault(const Json& j, const std::string& key, const Json& def);
bool hasKey(const Json& j, const std::string& key);

} // namespace json_utils

#endif // OJ_JSON_H

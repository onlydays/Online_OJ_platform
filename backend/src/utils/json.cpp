#include "json.h"
#include <stdexcept>

namespace json_utils {

std::string serialize(const Json& j) {
    return j.dump();
}

Json deserialize(const std::string& str) {
    return Json::parse(str);
}

Json errorResponse(int code, const std::string& message) {
    Json resp;
    resp["code"] = code;
    resp["message"] = message;
    return resp;
}

Json successResponse(const Json& data) {
    Json resp;
    resp["code"] = 0;
    resp["data"] = data;
    return resp;
}

Json successResponse(const std::string& key, const Json& value) {
    Json resp;
    resp["code"] = 0;
    resp[key] = value;
    return resp;
}

Json arrayResponse(const std::string& key, const Json& arr) {
    Json resp;
    resp["code"] = 0;
    resp[key] = arr;
    resp["total"] = arr.size();
    return resp;
}

Json getOrDefault(const Json& j, const std::string& key, const Json& def) {
    if (j.contains(key)) return j[key];
    return def;
}

bool hasKey(const Json& j, const std::string& key) {
    return j.contains(key);
}

} // namespace json_utils

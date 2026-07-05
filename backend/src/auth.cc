#include "auth.h"
#include "utils/config.h"
#include "utils/json.h"

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <stdexcept>
#include <cstring>

namespace auth {

std::string sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()),
           input.size(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setfill('0') << std::setw(2) << (int)hash[i];
    }
    return oss.str();
}

// ----  Base64URL 编码/解码 (JWT) ----

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64_encode(const std::string& input) {
    std::string result;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (result.size() % 4)
        result.push_back('=');
    return result;
}

static std::string base64_decode(const std::string& input) {
    std::string result;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;
    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            result.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return result;
}

static std::string base64url_encode(const std::string& input) {
    std::string b64 = base64_encode(input);
    for (char& c : b64) {
        if (c == '+') c = '-';
        else if (c == '/') c = '_';
    }
    b64.erase(std::find(b64.begin(), b64.end(), '='), b64.end());
    return b64;
}

static std::string base64url_decode(const std::string& input) {
    std::string b64 = input;
    for (char& c : b64) {
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
    }
    while (b64.size() % 4) b64.push_back('=');
    return base64_decode(b64);
}

// ----  HMAC-SHA256 ----

static std::string hmac_sha256(const std::string& data, const std::string& key) {
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    HMAC(EVP_sha256(), key.c_str(), key.size(),
         (const unsigned char*)data.c_str(), data.size(), result, &len);
    return std::string((char*)result, len);
}

// ----  JWT ----

std::string generateToken(int userId, const std::string& username,
                          const std::string& role) {
    auto& cfg = ConfigManager::getInstance();
    int expire = cfg.getInt("auth.jwt_expire_seconds", 86400);
    std::string secret = cfg.getString("auth.jwt_secret", "default-secret");

    auto now = std::chrono::system_clock::now();
    auto expTime = now + std::chrono::seconds(expire);
    auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
                       now.time_since_epoch()).count();
    auto exp_sec = std::chrono::duration_cast<std::chrono::seconds>(
                       expTime.time_since_epoch()).count();

    Json header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";

    Json payload;
    payload["user_id"] = userId;
    payload["username"] = username;
    payload["role"] = role;
    payload["iss"] = "oj-platform";
    payload["iat"] = now_sec;
    payload["exp"] = exp_sec;

    std::string headerB64 = base64url_encode(json_utils::serialize(header));
    std::string payloadB64 = base64url_encode(json_utils::serialize(payload));
    std::string signature = base64url_encode(hmac_sha256(
        headerB64 + "." + payloadB64, secret));

    return headerB64 + "." + payloadB64 + "." + signature;
}

std::optional<int> verifyToken(const std::string& token) {
    try {
        auto dot1 = token.find('.');
        auto dot2 = token.find('.', dot1 + 1);
        if (dot1 == std::string::npos || dot2 == std::string::npos)
            return std::nullopt;

        std::string headerB64 = token.substr(0, dot1);
        std::string payloadB64 = token.substr(dot1 + 1, dot2 - dot1 - 1);

        auto& cfg = ConfigManager::getInstance();
        std::string secret = cfg.getString("auth.jwt_secret", "default-secret");

        std::string expectedSig = base64url_encode(hmac_sha256(
            headerB64 + "." + payloadB64, secret));

        // Compare signatures manually to avoid timing attacks
        std::string actualSig = token.substr(dot2 + 1);
        if (expectedSig.size() != actualSig.size()) return std::nullopt;
        bool ok = true;
        for (size_t i = 0; i < expectedSig.size(); ++i) {
            if (expectedSig[i] != actualSig[i]) ok = false;
        }
        if (!ok) return std::nullopt;

        std::string payloadJson = base64url_decode(payloadB64);
        Json payload = json_utils::deserialize(payloadJson);

        // Check expiration
        auto now = std::chrono::system_clock::now();
        auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
                           now.time_since_epoch()).count();
        if (payload.contains("exp") && payload["exp"].get<int64_t>() < now_sec)
            return std::nullopt;

        if (!payload.contains("user_id")) return std::nullopt;
        return payload["user_id"].get<int>();
    } catch (...) {
        return std::nullopt;
    }
}

std::string extractUsername(const std::string& token) {
    try {
        auto dot1 = token.find('.');
        auto dot2 = token.find('.', dot1 + 1);
        std::string payloadB64 = token.substr(dot1 + 1, dot2 - dot1 - 1);
        Json payload = json_utils::deserialize(base64url_decode(payloadB64));
        return payload.value("username", "");
    } catch (...) {
        return "";
    }
}

std::string extractRole(const std::string& token) {
    try {
        auto dot1 = token.find('.');
        auto dot2 = token.find('.', dot1 + 1);
        std::string payloadB64 = token.substr(dot1 + 1, dot2 - dot1 - 1);
        Json payload = json_utils::deserialize(base64url_decode(payloadB64));
        return payload.value("role", "user");
    } catch (...) {
        return "user";
    }
}

} // namespace auth

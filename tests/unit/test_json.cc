#include <gtest/gtest.h>
#include "utils/json.h"

using json_utils::serialize;
using json_utils::deserialize;

// ========== 序列化与反序列化 ==========

TEST(JsonTest, SerializePrimitive) {
    Json j = 42;
    EXPECT_EQ(serialize(j), "42");
}

TEST(JsonTest, SerializeString) {
    Json j = "hello";
    EXPECT_EQ(serialize(j), "\"hello\"");
}

TEST(JsonTest, SerializeObject) {
    Json j;
    j["name"] = "alice";
    j["age"] = 25;
    std::string s = serialize(j);
    EXPECT_NE(s.find("\"name\""), std::string::npos);
    EXPECT_NE(s.find("\"alice\""), std::string::npos);
    EXPECT_NE(s.find("\"age\""), std::string::npos);
    EXPECT_NE(s.find("25"), std::string::npos);
}

TEST(JsonTest, SerializeArray) {
    Json arr = Json::array({1, 2, 3});
    std::string s = serialize(arr);
    EXPECT_NE(s.find("1"), std::string::npos);
    EXPECT_NE(s.find("3"), std::string::npos);
}

TEST(JsonTest, DeserializeNumber) {
    Json j = deserialize("42");
    EXPECT_EQ(j.get<int>(), 42);
}

TEST(JsonTest, DeserializeString) {
    Json j = deserialize("\"world\"");
    EXPECT_EQ(j.get<std::string>(), "world");
}

TEST(JsonTest, DeserializeObject) {
    Json j = deserialize(R"({"x": 10, "y": 20})");
    EXPECT_EQ(j["x"].get<int>(), 10);
    EXPECT_EQ(j["y"].get<int>(), 20);
}

TEST(JsonTest, DeserializeArray) {
    Json j = deserialize(R"([1, 2, 3])");
    EXPECT_TRUE(j.is_array());
    EXPECT_EQ(j.size(), 3);
    EXPECT_EQ(j[0].get<int>(), 1);
}

TEST(JsonTest, Roundtrip) {
    Json original;
    original["id"] = 1;
    original["title"] = "test";
    original["tags"] = Json::array({"dp", "math"});

    Json parsed = deserialize(serialize(original));
    EXPECT_EQ(parsed["id"].get<int>(), 1);
    EXPECT_EQ(parsed["title"].get<std::string>(), "test");
    EXPECT_EQ(parsed["tags"][0].get<std::string>(), "dp");
}

// ========== 错误响应 ==========

TEST(JsonTest, ErrorResponse) {
    Json resp = json_utils::errorResponse(404, "Not Found");
    EXPECT_EQ(resp["code"].get<int>(), 404);
    EXPECT_EQ(resp["message"].get<std::string>(), "Not Found");
}

// ========== 成功响应 ==========

TEST(JsonTest, SuccessResponseWithData) {
    Json data;
    data["result"] = "ok";
    Json resp = json_utils::successResponse(data);
    EXPECT_EQ(resp["code"].get<int>(), 0);
    EXPECT_EQ(resp["data"]["result"].get<std::string>(), "ok");
}

TEST(JsonTest, SuccessResponseWithKeyValue) {
    Json val = "hello";
    Json resp = json_utils::successResponse("msg", val);
    EXPECT_EQ(resp["code"].get<int>(), 0);
    EXPECT_EQ(resp["msg"].get<std::string>(), "hello");
}

// ========== 数组响应 ==========

TEST(JsonTest, ArrayResponse) {
    Json arr = Json::array();
    arr.push_back({{"id", 1}, {"title", "A"}});
    arr.push_back({{"id", 2}, {"title", "B"}});

    Json resp = json_utils::arrayResponse("problems", arr);
    EXPECT_EQ(resp["code"].get<int>(), 0);
    EXPECT_EQ(resp["total"].get<int>(), 2);
    EXPECT_EQ(resp["problems"][0]["id"].get<int>(), 1);
}

// ========== getOrDefault ==========

TEST(JsonTest, GetOrDefaultExisting) {
    Json j = {{"x", 100}};
    Json def = -1;
    Json val = json_utils::getOrDefault(j, "x", def);
    EXPECT_EQ(val.get<int>(), 100);
}

TEST(JsonTest, GetOrDefaultMissing) {
    Json j = {{"x", 100}};
    Json def = -1;
    Json val = json_utils::getOrDefault(j, "y", def);
    EXPECT_EQ(val.get<int>(), -1);
}

// ========== hasKey ==========

TEST(JsonTest, HasKeyTrue) {
    Json j = {{"name", "bob"}};
    EXPECT_TRUE(json_utils::hasKey(j, "name"));
}

TEST(JsonTest, HasKeyFalse) {
    Json j = {{"name", "bob"}};
    EXPECT_FALSE(json_utils::hasKey(j, "age"));
}

// ========== 空对象 ==========

TEST(JsonTest, EmptyObject) {
    Json j = Json::object();
    EXPECT_TRUE(j.is_object());
    EXPECT_TRUE(j.empty());
    EXPECT_EQ(serialize(j), "{}");
}

TEST(JsonTest, EmptyArray) {
    Json j = Json::array();
    EXPECT_TRUE(j.is_array());
    EXPECT_TRUE(j.empty());
    EXPECT_EQ(serialize(j), "[]");
}

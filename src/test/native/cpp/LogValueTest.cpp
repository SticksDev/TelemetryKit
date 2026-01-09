#include <gtest/gtest.h>
#include "telemetrykit/core/LogValue.h"

using namespace telemetrykit;

// Test primitive types
TEST(LogValueTest, BooleanValue) {
  LogValue value(true);
  EXPECT_EQ(value.GetType(), LogType::kBoolean);
  EXPECT_TRUE(value.Is<bool>());
  EXPECT_EQ(value.Get<bool>(), true);
  EXPECT_EQ(value.GetTypeString(), "boolean");
}

TEST(LogValueTest, Int64Value) {
  LogValue value(static_cast<int64_t>(42));
  EXPECT_EQ(value.GetType(), LogType::kInt64);
  EXPECT_TRUE(value.Is<int64_t>());
  EXPECT_EQ(value.Get<int64_t>(), 42);
  EXPECT_EQ(value.GetTypeString(), "int64");
}

TEST(LogValueTest, IntValue) {
  LogValue value(42);  // Regular int
  EXPECT_EQ(value.GetType(), LogType::kInt64);
  EXPECT_TRUE(value.Is<int64_t>());
  EXPECT_EQ(value.Get<int64_t>(), 42);
}

TEST(LogValueTest, FloatValue) {
  LogValue value(3.14f);
  EXPECT_EQ(value.GetType(), LogType::kFloat);
  EXPECT_TRUE(value.Is<float>());
  EXPECT_FLOAT_EQ(value.Get<float>(), 3.14f);
  EXPECT_EQ(value.GetTypeString(), "float");
}

TEST(LogValueTest, DoubleValue) {
  LogValue value(3.14159);
  EXPECT_EQ(value.GetType(), LogType::kDouble);
  EXPECT_TRUE(value.Is<double>());
  EXPECT_DOUBLE_EQ(value.Get<double>(), 3.14159);
  EXPECT_EQ(value.GetTypeString(), "double");
}

TEST(LogValueTest, StringValue) {
  LogValue value(std::string("Hello"));
  EXPECT_EQ(value.GetType(), LogType::kString);
  EXPECT_TRUE(value.Is<std::string>());
  EXPECT_EQ(value.Get<std::string>(), "Hello");
  EXPECT_EQ(value.GetTypeString(), "string");
}

TEST(LogValueTest, StringLiteralValue) {
  LogValue value("World");
  EXPECT_EQ(value.GetType(), LogType::kString);
  EXPECT_TRUE(value.Is<std::string>());
  EXPECT_EQ(value.Get<std::string>(), "World");
}

// Test array types
TEST(LogValueTest, BooleanArray) {
  std::vector<bool> data = {true, false, true};
  LogValue value(data);
  EXPECT_EQ(value.GetType(), LogType::kBooleanArray);
  EXPECT_TRUE(value.Is<std::vector<bool>>());
  auto result = value.Get<std::vector<bool>>();
  EXPECT_EQ(result.size(), 3u);
  EXPECT_EQ(result[0], true);
  EXPECT_EQ(result[1], false);
  EXPECT_EQ(result[2], true);
  EXPECT_EQ(value.GetTypeString(), "boolean[]");
}

TEST(LogValueTest, Int64Array) {
  std::vector<int64_t> data = {1, 2, 3, 4, 5};
  LogValue value(data);
  EXPECT_EQ(value.GetType(), LogType::kInt64Array);
  EXPECT_TRUE(value.Is<std::vector<int64_t>>());
  EXPECT_EQ(value.Get<std::vector<int64_t>>(), data);
  EXPECT_EQ(value.GetTypeString(), "int64[]");
}

TEST(LogValueTest, IntArray) {
  std::vector<int> data = {1, 2, 3};
  LogValue value(data);
  EXPECT_EQ(value.GetType(), LogType::kInt64Array);
  EXPECT_TRUE(value.Is<std::vector<int64_t>>());
  auto result = value.Get<std::vector<int64_t>>();
  EXPECT_EQ(result.size(), 3u);
  EXPECT_EQ(result[0], 1);
  EXPECT_EQ(result[1], 2);
  EXPECT_EQ(result[2], 3);
}

TEST(LogValueTest, FloatArray) {
  std::vector<float> data = {1.1f, 2.2f, 3.3f};
  LogValue value(data);
  EXPECT_EQ(value.GetType(), LogType::kFloatArray);
  EXPECT_TRUE(value.Is<std::vector<float>>());
  EXPECT_EQ(value.Get<std::vector<float>>(), data);
  EXPECT_EQ(value.GetTypeString(), "float[]");
}

TEST(LogValueTest, DoubleArray) {
  std::vector<double> data = {1.1, 2.2, 3.3};
  LogValue value(data);
  EXPECT_EQ(value.GetType(), LogType::kDoubleArray);
  EXPECT_TRUE(value.Is<std::vector<double>>());
  EXPECT_EQ(value.Get<std::vector<double>>(), data);
  EXPECT_EQ(value.GetTypeString(), "double[]");
}

TEST(LogValueTest, StringArray) {
  std::vector<std::string> data = {"one", "two", "three"};
  LogValue value(data);
  EXPECT_EQ(value.GetType(), LogType::kStringArray);
  EXPECT_TRUE(value.Is<std::vector<std::string>>());
  EXPECT_EQ(value.Get<std::vector<std::string>>(), data);
  EXPECT_EQ(value.GetTypeString(), "string[]");
}

// Test raw data
TEST(LogValueTest, RawData) {
  std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
  LogValue value(data, false);  // Not a struct
  EXPECT_EQ(value.GetType(), LogType::kRaw);
  EXPECT_TRUE(value.Is<std::vector<uint8_t>>());
  EXPECT_EQ(value.Get<std::vector<uint8_t>>(), data);
  EXPECT_EQ(value.GetTypeString(), "raw");
}

// Test struct data
TEST(LogValueTest, StructData) {
  std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
  std::string typeStr = "Pose2d";  // Force use of string_view constructor
  LogValue value(data, typeStr);
  EXPECT_EQ(value.GetType(), LogType::kStruct);
  EXPECT_TRUE(value.Is<std::vector<uint8_t>>());
  EXPECT_EQ(value.Get<std::vector<uint8_t>>(), data);
  EXPECT_EQ(value.GetTypeString(), "Pose2d");
}

TEST(LogValueTest, StructArrayData) {
  std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
  LogValue value(data, "SwerveModuleState", true);
  EXPECT_EQ(value.GetType(), LogType::kStructArray);
  EXPECT_TRUE(value.Is<std::vector<uint8_t>>());
  EXPECT_EQ(value.Get<std::vector<uint8_t>>(), data);
  EXPECT_EQ(value.GetTypeString(), "SwerveModuleState");
}

// Test type checking
TEST(LogValueTest, TypeChecking) {
  LogValue value(42);
  EXPECT_TRUE(value.Is<int64_t>());
  EXPECT_FALSE(value.Is<double>());
  EXPECT_FALSE(value.Is<std::string>());
}

// Test GetOr fallback
TEST(LogValueTest, GetOrFallback) {
  LogValue value(42);
  EXPECT_EQ(value.GetOr<int64_t>(0), 42);
  EXPECT_DOUBLE_EQ(value.GetOr<double>(3.14), 3.14);  // Wrong type, returns default
}

// Test equality
TEST(LogValueTest, EqualityPrimitives) {
  LogValue v1(42);
  LogValue v2(42);
  LogValue v3(43);

  EXPECT_EQ(v1, v2);
  EXPECT_NE(v1, v3);
}

TEST(LogValueTest, EqualityArrays) {
  std::vector<double> data1 = {1.1, 2.2, 3.3};
  std::vector<double> data2 = {1.1, 2.2, 3.3};
  std::vector<double> data3 = {1.1, 2.2, 4.4};

  LogValue v1(data1);
  LogValue v2(data2);
  LogValue v3(data3);

  EXPECT_EQ(v1, v2);
  EXPECT_NE(v1, v3);
}

TEST(LogValueTest, EqualityStructs) {
  std::vector<uint8_t> data1 = {0x01, 0x02};
  std::vector<uint8_t> data2 = {0x01, 0x02};
  std::vector<uint8_t> data3 = {0x01, 0x03};

  std::string type1 = "Pose2d";
  std::string type2 = "Transform2d";

  LogValue v1(data1, type1);
  LogValue v2(data2, type1);
  LogValue v3(data3, type1);
  LogValue v4(data1, type2);  // Different type string

  EXPECT_EQ(v1, v2);
  EXPECT_NE(v1, v3);  // Different data
  EXPECT_NE(v1, v4);  // Different type string
}

// Test copy and move
TEST(LogValueTest, CopyConstructor) {
  LogValue v1(42);
  LogValue v2(v1);

  EXPECT_EQ(v2.GetType(), LogType::kInt64);
  EXPECT_EQ(v2.Get<int64_t>(), 42);
}

TEST(LogValueTest, MoveConstructor) {
  std::vector<double> data = {1.1, 2.2, 3.3};
  LogValue v1(data);
  LogValue v2(std::move(v1));

  EXPECT_EQ(v2.GetType(), LogType::kDoubleArray);
  EXPECT_EQ(v2.Get<std::vector<double>>(), data);
}

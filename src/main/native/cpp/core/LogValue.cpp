#include "telemetrykit/core/LogValue.h"

namespace telemetrykit {

// Default constructor
LogValue::LogValue()
  : m_type(LogType::kBoolean), m_value(false) {}

// Primitive constructors
LogValue::LogValue(bool value)
  : m_type(LogType::kBoolean), m_value(value) {}

LogValue::LogValue(int64_t value)
  : m_type(LogType::kInt64), m_value(value) {}

LogValue::LogValue(int value)
  : m_type(LogType::kInt64), m_value(static_cast<int64_t>(value)) {}

LogValue::LogValue(float value)
  : m_type(LogType::kFloat), m_value(value) {}

LogValue::LogValue(double value)
  : m_type(LogType::kDouble), m_value(value) {}

LogValue::LogValue(const std::string& value)
  : m_type(LogType::kString), m_value(value) {}

LogValue::LogValue(const char* value)
  : m_type(LogType::kString), m_value(std::string(value)) {}

// Array constructors
LogValue::LogValue(const std::vector<bool>& value)
  : m_type(LogType::kBooleanArray), m_value(value) {}

LogValue::LogValue(const std::vector<int64_t>& value)
  : m_type(LogType::kInt64Array), m_value(value) {}

LogValue::LogValue(const std::vector<int>& value)
  : m_type(LogType::kInt64Array) {
  std::vector<int64_t> converted(value.begin(), value.end());
  m_value = converted;
}

LogValue::LogValue(const std::vector<float>& value)
  : m_type(LogType::kFloatArray), m_value(value) {}

LogValue::LogValue(const std::vector<double>& value)
  : m_type(LogType::kDoubleArray), m_value(value) {}

LogValue::LogValue(const std::vector<std::string>& value)
  : m_type(LogType::kStringArray), m_value(value) {}

// Raw binary constructor
LogValue::LogValue(const std::vector<uint8_t>& value, bool isStruct)
  : m_type(isStruct ? LogType::kStruct : LogType::kRaw), m_value(value) {}

// Struct constructor with type string
LogValue::LogValue(const std::vector<uint8_t>& data, std::string_view typeString)
  : m_type(LogType::kStruct), m_value(data), m_typeString(typeString) {}

// Struct constructor with type string and schema
LogValue::LogValue(const std::vector<uint8_t>& data, std::string_view typeString,
                   std::span<const uint8_t> schema)
  : m_type(LogType::kStruct), m_value(data), m_typeString(typeString),
    m_schema(schema.begin(), schema.end()) {}

// Struct constructor with all schemas (including nested)
LogValue::LogValue(const std::vector<uint8_t>& data, std::string_view typeString,
                   std::span<const uint8_t> schema,
                   const std::unordered_map<std::string, std::vector<uint8_t>>& nestedSchemas)
  : m_type(LogType::kStruct), m_value(data), m_typeString(typeString),
    m_schema(schema.begin(), schema.end()), m_nestedSchemas(nestedSchemas) {}

// Struct array constructor
LogValue::LogValue(const std::vector<uint8_t>& data, std::string_view typeString, bool isArray)
  : m_type(isArray ? LogType::kStructArray : LogType::kStruct),
    m_value(data),
    m_typeString(typeString) {}

// Struct array constructor with schema
LogValue::LogValue(const std::vector<uint8_t>& data, std::string_view typeString,
                   std::span<const uint8_t> schema, bool isArray)
  : m_type(isArray ? LogType::kStructArray : LogType::kStruct),
    m_value(data),
    m_typeString(typeString),
    m_schema(schema.begin(), schema.end()) {}

// Struct array constructor with all schemas (including nested)
LogValue::LogValue(const std::vector<uint8_t>& data, std::string_view typeString,
                   std::span<const uint8_t> schema,
                   const std::unordered_map<std::string, std::vector<uint8_t>>& nestedSchemas,
                   bool isArray)
  : m_type(isArray ? LogType::kStructArray : LogType::kStruct),
    m_value(data),
    m_typeString(typeString),
    m_schema(schema.begin(), schema.end()),
    m_nestedSchemas(nestedSchemas) {}

std::string LogValue::GetTypeString() const {
  // For structs, return the custom type string
  if (m_type == LogType::kStruct || m_type == LogType::kStructArray) {
    return m_typeString;
  }

  // For primitives, return the type name
  switch (m_type) {
    case LogType::kBoolean:
      return "boolean";
    case LogType::kInt64:
      return "int64";
    case LogType::kFloat:
      return "float";
    case LogType::kDouble:
      return "double";
    case LogType::kString:
      return "string";
    case LogType::kBooleanArray:
      return "boolean[]";
    case LogType::kInt64Array:
      return "int64[]";
    case LogType::kFloatArray:
      return "float[]";
    case LogType::kDoubleArray:
      return "double[]";
    case LogType::kStringArray:
      return "string[]";
    case LogType::kRaw:
      return "raw";
    default:
      return "unknown";
  }
}

bool LogValue::operator==(const LogValue& other) const {
  // Type must match
  if (m_type != other.m_type) {
    return false;
  }

  // For structs, also compare type string
  if ((m_type == LogType::kStruct || m_type == LogType::kStructArray) &&
      m_typeString != other.m_typeString) {
    return false;
  }

  // Compare the variant values
  return m_value == other.m_value;
}

}  // namespace telemetrykit

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <wpi/struct/Struct.h>

namespace telemetrykit {

/**
 * Enum representing the runtime type of a LogValue.
 */
enum class LogType {
  kBoolean,
  kInt64,
  kFloat,
  kDouble,
  kString,
  kBooleanArray,
  kInt64Array,
  kFloatArray,
  kDoubleArray,
  kStringArray,
  kRaw,          // For arbitrary binary data
  kStruct,       // WPILib structs (Pose2d, etc.)
  kStructArray
};

/**
 * LogValue - Type-safe value wrapper using std::variant.
 *
 * Stores values with runtime type information while maintaining compile-time
 * type safety. Supports all WPILOG primitive types, arrays, and WPILib structs.
 */
class LogValue {
 public:
  // Variant type holding all possible value types
  using ValueVariant = std::variant<
    bool,
    int64_t,
    float,
    double,
    std::string,
    std::vector<bool>,
    std::vector<int64_t>,
    std::vector<float>,
    std::vector<double>,
    std::vector<std::string>,
    std::vector<uint8_t>  // Raw + struct data
  >;

  // Default constructor (for use in containers)
  LogValue();

  // Constructors for primitive types
  LogValue(bool value);
  LogValue(int64_t value);
  LogValue(int value);  // Convenience for int
  LogValue(float value);
  LogValue(double value);
  LogValue(const std::string& value);
  LogValue(const char* value);  // Convenience for string literals

  // Constructors for array types
  LogValue(const std::vector<bool>& value);
  LogValue(const std::vector<int64_t>& value);
  LogValue(const std::vector<int>& value);  // Convenience
  LogValue(const std::vector<float>& value);
  LogValue(const std::vector<double>& value);
  LogValue(const std::vector<std::string>& value);

  // Constructor for struct data with type string (MUST come before bool constructor)
  LogValue(const std::vector<uint8_t>& data, std::string_view typeString);

  // Constructor for struct data with type string and schema
  LogValue(const std::vector<uint8_t>& data, std::string_view typeString,
           std::span<const uint8_t> schema);

  // Constructor for struct data with all schemas (including nested)
  LogValue(const std::vector<uint8_t>& data, std::string_view typeString,
           std::span<const uint8_t> schema,
           const std::unordered_map<std::string, std::vector<uint8_t>>& nestedSchemas);

  // Constructor for struct arrays
  LogValue(const std::vector<uint8_t>& data, std::string_view typeString, bool isArray);

  // Constructor for struct arrays with schema
  LogValue(const std::vector<uint8_t>& data, std::string_view typeString,
           std::span<const uint8_t> schema, bool isArray);

  // Constructor for struct arrays with all schemas (including nested)
  LogValue(const std::vector<uint8_t>& data, std::string_view typeString,
           std::span<const uint8_t> schema,
           const std::unordered_map<std::string, std::vector<uint8_t>>& nestedSchemas,
           bool isArray);

  // Constructor for raw binary data (no default parameter to avoid ambiguity)
  LogValue(const std::vector<uint8_t>& value, bool isStruct);

  // Copy and move
  LogValue(const LogValue& other) = default;
  LogValue(LogValue&& other) = default;
  LogValue& operator=(const LogValue& other) = default;
  LogValue& operator=(LogValue&& other) = default;

  /**
   * Get the runtime type of this value.
   */
  LogType GetType() const { return m_type; }

  /**
   * Get the type string for WPILOG/NT4.
   * Returns the LogType name for primitives, or custom type string for structs.
   */
  std::string GetTypeString() const;

  /**
   * Get the underlying variant value.
   */
  const ValueVariant& GetValue() const { return m_value; }

  /**
   * Check if this LogValue holds a specific type T.
   *
   * Example:
   *   if (value.Is<double>()) { ... }
   */
  template<typename T>
  bool Is() const {
    return std::holds_alternative<T>(m_value);
  }

  /**
   * Get the value as type T.
   * Throws std::bad_variant_access if type doesn't match.
   *
   * Example:
   *   double d = value.Get<double>();
   */
  template<typename T>
  const T& Get() const {
    return std::get<T>(m_value);
  }

  /**
   * Get the value as type T, or return a default if type doesn't match.
   *
   * Example:
   *   double d = value.GetOr<double>(0.0);
   */
  template<typename T>
  T GetOr(const T& defaultValue) const {
    try {
      return std::get<T>(m_value);
    } catch (const std::bad_variant_access&) {
      return defaultValue;
    }
  }

  /**
   * Equality comparison.
   */
  bool operator==(const LogValue& other) const;
  bool operator!=(const LogValue& other) const { return !(*this == other); }

 private:
  LogType m_type;
  ValueVariant m_value;
  std::string m_typeString;  // For struct types (e.g., "Pose2d")
  std::vector<uint8_t> m_schema;  // For struct schema bytes (NT4 schema publishing)
  std::unordered_map<std::string, std::vector<uint8_t>> m_nestedSchemas;  // Nested struct schemas

 public:
  /**
   * Get the schema bytes for struct types.
   * Returns empty span for non-struct types.
   */
  std::span<const uint8_t> GetSchema() const { return m_schema; }

  /**
   * Get all nested schemas (including the main schema).
   * Returns map of typeName -> schema bytes.
   */
  const std::unordered_map<std::string, std::vector<uint8_t>>& GetAllSchemas() const {
    return m_nestedSchemas;
  }
};

/**
 * Helper function to create a LogValue from a WPILib struct.
 *
 * Example:
 *   frc::Pose2d pose{...};
 *   LogValue value = MakeStructValue(pose);
 */
template<typename T>
LogValue MakeStructValue(const T& structValue) {
  // Get the struct descriptor (all WPILib structs have a static 'struct' member)
  auto& descriptor = T::struct_type;

  // Serialize to bytes
  std::vector<uint8_t> buffer(descriptor.GetSize());
  descriptor.Pack(buffer, structValue);

  // Get schema bytes
  auto schemaBytes = wpi::GetStructSchemaBytes<T>();

  // Collect all schemas (including nested structs like Translation2d in Pose2d)
  std::unordered_map<std::string, std::vector<uint8_t>> allSchemas;
  wpi::ForEachStructSchema<T>([&allSchemas](std::string_view name, std::string_view schema) {
    allSchemas[std::string(name)] = std::vector<uint8_t>(schema.begin(), schema.end());
  });

  // Create LogValue with type string, schema, and all nested schemas
  return LogValue(buffer, std::string(descriptor.GetTypeName()), schemaBytes, allSchemas);
}

/**
 * Helper function to create a LogValue from an array of WPILib structs.
 */
template<typename T>
LogValue MakeStructArrayValue(std::span<const T> structArray) {
  auto& descriptor = T::struct_type;

  // Serialize array to bytes
  std::vector<uint8_t> buffer(descriptor.GetSize() * structArray.size());
  for (size_t i = 0; i < structArray.size(); ++i) {
    std::span<uint8_t> slice(buffer.data() + i * descriptor.GetSize(),
                             descriptor.GetSize());
    descriptor.Pack(slice, structArray[i]);
  }

  // Get schema bytes
  auto schemaBytes = wpi::GetStructSchemaBytes<T>();

  // Collect all schemas (including nested structs)
  std::unordered_map<std::string, std::vector<uint8_t>> allSchemas;
  wpi::ForEachStructSchema<T>([&allSchemas](std::string_view name, std::string_view schema) {
    allSchemas[std::string(name)] = std::vector<uint8_t>(schema.begin(), schema.end());
  });

  // Create LogValue with type string, schema, all nested schemas, and array flag
  return LogValue(buffer, std::string(descriptor.GetTypeName()), schemaBytes, allSchemas, true);
}

}  // namespace telemetrykit

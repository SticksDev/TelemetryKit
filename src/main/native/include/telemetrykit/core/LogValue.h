#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <networktables/NetworkTableInstance.h>
#include <wpi/struct/Struct.h>

namespace tkit {

/**
 * Runtime tag for LogValue contents.
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
  kRaw,          // arbitrary bytes
  kStruct,       // packed WPILib struct bytes + schema/type name
  kStructArray
};

/**
 * A tagged value used by the logging pipeline.
 *
 * Backed by a std::variant for the common primitive/array cases. Structs are
 * stored as packed bytes with an associated type name and schema data.
 */
class LogValue {
 public:
  // All supported stored value shapes.
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
    std::vector<uint8_t>  // raw + packed struct data
  >;

  // For containers / default init
  LogValue();

  // Primitive constructors
  LogValue(bool value);
  LogValue(int64_t value);
  LogValue(int value);
  LogValue(float value);
  LogValue(double value);
  LogValue(const std::string& value);
  LogValue(const char* value);

  // Array constructors
  LogValue(const std::vector<bool>& value);
  LogValue(const std::vector<int64_t>& value);
  LogValue(const std::vector<int>& value);
  LogValue(const std::vector<float>& value);
  LogValue(const std::vector<double>& value);
  LogValue(const std::vector<std::string>& value);

  // Packed struct bytes + type name
  // (kept separate from raw to avoid overload ambiguity)
  LogValue(const std::vector<uint8_t>& data, std::string_view typeString);

  // Struct bytes + type + schema
  LogValue(const std::vector<uint8_t>& data, std::string_view typeString,
           std::span<const uint8_t> schema);

  // Struct bytes + type + schema + any nested schemas
  LogValue(const std::vector<uint8_t>& data, std::string_view typeString,
           std::span<const uint8_t> schema,
           const std::unordered_map<std::string, std::vector<uint8_t>>& nestedSchemas);

  // Struct array variants
  LogValue(const std::vector<uint8_t>& data, std::string_view typeString, bool isArray);
  LogValue(const std::vector<uint8_t>& data, std::string_view typeString,
           std::span<const uint8_t> schema, bool isArray);
  LogValue(const std::vector<uint8_t>& data, std::string_view typeString,
           std::span<const uint8_t> schema,
           const std::unordered_map<std::string, std::vector<uint8_t>>& nestedSchemas,
           bool isArray);

  // Raw bytes (not a struct)
  LogValue(const std::vector<uint8_t>& value, bool isStruct);

  // Copy/move
  LogValue(const LogValue& other) = default;
  LogValue(LogValue&& other) = default;
  LogValue& operator=(const LogValue& other) = default;
  LogValue& operator=(LogValue&& other) = default;

  /// Type tag.
  LogType GetType() const { return m_type; }

  /// WPILOG/NT type string (primitive name or struct type name).
  std::string GetTypeString() const;

  /// Underlying stored value.
  const ValueVariant& GetValue() const { return m_value; }

  /// True if the variant currently holds T.
  template<typename T>
  bool Is() const {
    return std::holds_alternative<T>(m_value);
  }

  /// Returns the stored T (throws if wrong type).
  template<typename T>
  const T& Get() const {
    return std::get<T>(m_value);
  }

  /// Returns stored T or a default if the type doesn't match.
  template<typename T>
  T GetOr(const T& defaultValue) const {
    try {
      return std::get<T>(m_value);
    } catch (const std::bad_variant_access&) {
      return defaultValue;
    }
  }

  bool operator==(const LogValue& other) const;
  bool operator!=(const LogValue& other) const { return !(*this == other); }

 private:
  LogType m_type;
  ValueVariant m_value;

  // Struct metadata
  std::string m_typeString;
  std::vector<uint8_t> m_schema;
  std::unordered_map<std::string, std::vector<uint8_t>> m_nestedSchemas;

  // Type-erased schema registration function (captures T at construction)
  using SchemaRegistrar = std::function<void(nt::NetworkTableInstance&)>;
  SchemaRegistrar m_schemaRegistrar;

 public:
  /// Schema bytes for struct types (empty for non-struct).
  std::span<const uint8_t> GetSchema() const { return m_schema; }

  /// Map of type name -> schema bytes (includes nested structs).
  const std::unordered_map<std::string, std::vector<uint8_t>>& GetAllSchemas() const {
    return m_nestedSchemas;
  }

  /// Registers struct schema with NetworkTables (call once per type).
  void RegisterSchema(nt::NetworkTableInstance& inst) const {
    if (m_schemaRegistrar) {
      m_schemaRegistrar(inst);
    }
  }

  /// Returns true if this value has a schema registrar.
  bool HasSchemaRegistrar() const { return static_cast<bool>(m_schemaRegistrar); }

  /// Sets the schema registrar (used by MakeStructValue/MakeStructArrayValue).
  void SetSchemaRegistrar(SchemaRegistrar registrar) {
    m_schemaRegistrar = std::move(registrar);
  }
};

/**
 * Packs a WPILib struct into a LogValue (bytes + type name + schema info).
 */
template<wpi::StructSerializable T>
LogValue MakeStructValue(const T& structValue) {
  using S = wpi::Struct<typename std::remove_cvref_t<T>>;

  std::vector<uint8_t> buffer(S::GetSize());
  S::Pack(buffer, structValue);

  auto schemaBytes = wpi::GetStructSchemaBytes<T>();

  std::unordered_map<std::string, std::vector<uint8_t>> allSchemas;
  wpi::ForEachStructSchema<T>([&allSchemas](std::string_view name, std::string_view schema) {
    allSchemas[std::string(name)] = std::vector<uint8_t>(schema.begin(), schema.end());
  });

  // Use struct: prefixed type string for NT4
  std::string typeString = wpi::GetStructTypeString<T>();

  LogValue value(buffer, typeString, schemaBytes, allSchemas);
  value.SetSchemaRegistrar([](nt::NetworkTableInstance& inst) {
    inst.AddStructSchema<T>();
  });
  
  return value;
}

/**
 * Packs an array of WPILib structs into a LogValue.
 */
template<wpi::StructSerializable T>
LogValue MakeStructArrayValue(std::span<const T> structArray) {
  using S = wpi::Struct<typename std::remove_cvref_t<T>>;

  std::vector<uint8_t> buffer(S::GetSize() * structArray.size());
  for (size_t i = 0; i < structArray.size(); ++i) {
    std::span<uint8_t> slice(buffer.data() + i * S::GetSize(), S::GetSize());
    S::Pack(slice, structArray[i]);
  }

  auto schemaBytes = wpi::GetStructSchemaBytes<T>();

  std::unordered_map<std::string, std::vector<uint8_t>> allSchemas;
  wpi::ForEachStructSchema<T>([&allSchemas](std::string_view name, std::string_view schema) {
    allSchemas[std::string(name)] = std::vector<uint8_t>(schema.begin(), schema.end());
  });

  // Use struct: prefixed type string for NT4 arrays (e.g., "struct:Pose2d[]")
  std::string typeString = wpi::GetStructTypeString<T>();
  typeString += "[]";

  LogValue value(buffer, typeString, schemaBytes, allSchemas, true);
  value.SetSchemaRegistrar([](nt::NetworkTableInstance& inst) {
    inst.AddStructSchema<T>();
  });
  return value;
}

/**
 * Packs an array of WPILib structs into a LogValue (vector overload).
 */
template<wpi::StructSerializable T>
LogValue MakeStructArrayValue(const std::vector<T>& structArray) {
  return MakeStructArrayValue<T>(std::span<const T>(structArray));
}

}
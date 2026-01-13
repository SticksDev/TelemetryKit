#pragma once

#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "telemetrykit/core/LogValue.h"

namespace tkit {

/**
 * LogTable - Flat key-value storage with hierarchical "/" prefixes.
 *
 * LogTable provides a simple, thread-safe storage mechanism for log data.
 * Uses a flat std::unordered_map with "/" separated keys to create a virtual
 * hierarchy (similar to PyKit's approach).
 *
 * Example:
 *   LogTable table;
 *   table.Put("/Drivetrain/LeftMotor/Velocity", 3.5);
 *   table.Put("/Drivetrain/RightMotor/Velocity", 3.3);
 *
 *   auto driveTable = table.GetSubtable("/Drivetrain");
 *   driveTable.Put("DesiredSpeed", 5.0);  // Becomes "/Drivetrain/DesiredSpeed"
 */
class LogTable {
 public:
  /**
   * Create an empty LogTable with no prefix.
   */
  LogTable();

  /**
   * Create a LogTable with a prefix (used for subtables).
   */
  explicit LogTable(std::string_view prefix);

  /**
   * Store a LogValue with the given key.
   *
   * If a prefix is set, it's prepended to the key.
   */
  void Put(std::string_view key, const LogValue& value);

  /**
   * Store a LogValue with the given key and unit metadata.
   *
   * Unit metadata is used by AdvantageScope for unit-aware graphing.
   * Common units: "meters", "radians", "m/s", "volts", "amps"
   */
  void Put(std::string_view key, const LogValue& value, std::string_view unit);

  /**
   * Template convenience method to store any supported type.
   *
   * Example:
   *   table.Put("/Speed", 3.5);
   *   table.Put("/Enabled", true);
   *   table.Put("/Name", "Robot");
   */
  template<typename T>
  void Put(std::string_view key, const T& value) {
    Put(key, LogValue(value));
  }

  /**
   * Template convenience method to store a value with unit metadata.
   *
   * Example:
   *   table.Put("/Speed", 3.5, "m/s");
   *   table.Put("/Angle", 1.57, "radians");
   */
  template<typename T>
  void Put(std::string_view key, const T& value, std::string_view unit) {
    Put(key, LogValue(value), unit);
  }

  /**
   * Retrieve a value by key.
   *
   * Returns std::nullopt if the key doesn't exist.
   */
  std::optional<LogValue> Get(std::string_view key) const;

  /**
   * Check if a key exists in the table.
   */
  bool Contains(std::string_view key) const;

  /**
   * Remove a key from the table.
   */
  void Remove(std::string_view key);

  /**
   * Clear all entries from the table.
   */
  void Clear();

  /**
   * Get the number of entries in the table.
   */
  size_t Size() const;

  /**
   * Check if the table is empty.
   */
  bool IsEmpty() const;

  /**
   * Create a subtable with the given prefix.
   *
   * A subtable is a view that automatically prepends the prefix to all keys.
   * The subtable shares the same underlying storage.
   *
   * Example:
   *   auto gyroTable = rootTable.GetSubtable("/Gyro");
   *   gyroTable.Put("Yaw", 45.0);  // Stored as "/Gyro/Yaw"
   */
  LogTable GetSubtable(std::string_view prefix) const;

  /**
   * Get all entries in the table (or under the current prefix).
   *
   * Thread-safe: Returns a copy of the entries.
   */
  std::unordered_map<std::string, LogValue> GetAllEntries() const;

  /**
   * Get all keys in the table (or under the current prefix).
   */
  std::vector<std::string> GetKeys() const;

  /**
   * Get the unit metadata for a key.
   *
   * Returns empty string if no unit is set.
   */
  std::string GetUnit(std::string_view key) const;

  /**
   * Get all unit metadata in the table.
   *
   * Returns map of key -> unit string.
   */
  std::unordered_map<std::string, std::string> GetAllUnits() const;

  /**
   * Check if a value has changed compared to what's in the table.
   *
   * Useful for change detection in receivers.
   * Returns true if:
   *   - Key doesn't exist yet
   *   - Value is different from stored value
   */
  bool HasChanged(std::string_view key, const LogValue& newValue) const;

  /**
   * Get the current prefix for this table.
   */
  std::string GetPrefix() const { return m_prefix; }

 private:
  /**
   * Build the full key by combining prefix + key.
   * Ensures proper "/" formatting.
   */
  std::string BuildKey(std::string_view key) const;

  /**
   * Normalize a prefix to ensure it starts with "/" and doesn't end with "/".
   */
  static std::string NormalizePrefix(std::string_view prefix);

  std::string m_prefix;
  std::shared_ptr<std::unordered_map<std::string, LogValue>> m_entries;
  std::shared_ptr<std::unordered_map<std::string, std::string>> m_units;  // key -> unit metadata
  mutable std::shared_ptr<std::shared_mutex> m_mutex;
};

}  // namespace tkit

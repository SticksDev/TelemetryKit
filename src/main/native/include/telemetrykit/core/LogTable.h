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
 * Thread-safe key/value store for log data.
 *
 * Keys are plain strings; "/" is used by convention to group values. Subtables
 * are lightweight views that prepend a prefix to keys and share the same backing
 * storage.
 */
class LogTable {
 public:
  /// Empty table with no prefix.
  LogTable();

  /// Table view with a fixed prefix (used for subtables).
  explicit LogTable(std::string_view prefix);

  /// Stores a value at key (prefix is applied if set).
  void Put(std::string_view key, const LogValue& value);

  /// Stores a value and optional unit metadata.
  void Put(std::string_view key, const LogValue& value, std::string_view unit);

  // Convenience overloads
  template<typename T>
  void Put(std::string_view key, const T& value) {
    Put(key, LogValue(value));
  }

  template<typename T>
  void Put(std::string_view key, const T& value, std::string_view unit) {
    Put(key, LogValue(value), unit);
  }

  /// Returns the stored value, or std::nullopt if missing.
  std::optional<LogValue> Get(std::string_view key) const;

  /// True if key exists.
  bool Contains(std::string_view key) const;

  /// Removes key if present.
  void Remove(std::string_view key);

  /// Clears all entries (and units).
  void Clear();

  size_t Size() const;
  bool IsEmpty() const;

  /**
   * Returns a table view rooted at prefix (shares backing maps).
   */
  LogTable GetSubtable(std::string_view prefix) const;

  /**
   * Snapshot of entries under the current prefix.
   */
  std::unordered_map<std::string, LogValue> GetAllEntries() const;

  /// Keys under the current prefix.
  std::vector<std::string> GetKeys() const;

  /// Unit string for key, or empty if none.
  std::string GetUnit(std::string_view key) const;

  /// Snapshot of all unit metadata.
  std::unordered_map<std::string, std::string> GetAllUnits() const;

  /**
   * Returns true if key is missing or the stored value != newValue.
   * Handy for receivers that want to skip unchanged outputs.
   */
  bool HasChanged(std::string_view key, const LogValue& newValue) const;

  std::string GetPrefix() const { return m_prefix; }

 private:
  /// Combines prefix + key with consistent '/' handling.
  std::string BuildKey(std::string_view key) const;

  /// Normalizes prefixes to start with '/' and not end with '/'.
  static std::string NormalizePrefix(std::string_view prefix);

  std::string m_prefix;
  std::shared_ptr<std::unordered_map<std::string, LogValue>> m_entries;
  std::shared_ptr<std::unordered_map<std::string, std::string>> m_units;
  mutable std::shared_ptr<std::shared_mutex> m_mutex;
};

}

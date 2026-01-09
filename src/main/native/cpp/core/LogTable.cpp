#include "telemetrykit/core/LogTable.h"

#include <mutex>

namespace telemetrykit {

LogTable::LogTable()
  : m_prefix(""),
    m_entries(std::make_shared<std::unordered_map<std::string, LogValue>>()),
    m_mutex(std::make_shared<std::shared_mutex>()) {}

LogTable::LogTable(std::string_view prefix)
  : m_prefix(NormalizePrefix(prefix)),
    m_entries(std::make_shared<std::unordered_map<std::string, LogValue>>()),
    m_mutex(std::make_shared<std::shared_mutex>()) {}

void LogTable::Put(std::string_view key, const LogValue& value) {
  std::string fullKey = BuildKey(key);
  std::unique_lock lock(*m_mutex);
  (*m_entries)[fullKey] = value;
}

std::optional<LogValue> LogTable::Get(std::string_view key) const {
  std::string fullKey = BuildKey(key);
  std::shared_lock lock(*m_mutex);

  auto it = m_entries->find(fullKey);
  if (it != m_entries->end()) {
    return it->second;
  }
  return std::nullopt;
}

bool LogTable::Contains(std::string_view key) const {
  std::string fullKey = BuildKey(key);
  std::shared_lock lock(*m_mutex);
  return m_entries->find(fullKey) != m_entries->end();
}

void LogTable::Remove(std::string_view key) {
  std::string fullKey = BuildKey(key);
  std::unique_lock lock(*m_mutex);
  m_entries->erase(fullKey);
}

void LogTable::Clear() {
  std::unique_lock lock(*m_mutex);

  // If we have a prefix, only clear entries with that prefix
  if (!m_prefix.empty()) {
    auto it = m_entries->begin();
    while (it != m_entries->end()) {
      if (it->first.starts_with(m_prefix)) {
        it = m_entries->erase(it);
      } else {
        ++it;
      }
    }
  } else {
    // No prefix, clear everything
    m_entries->clear();
  }
}

size_t LogTable::Size() const {
  std::shared_lock lock(*m_mutex);

  // If we have a prefix, count only entries with that prefix
  if (!m_prefix.empty()) {
    size_t count = 0;
    for (const auto& [key, value] : *m_entries) {
      if (key.starts_with(m_prefix)) {
        ++count;
      }
    }
    return count;
  }

  return m_entries->size();
}

bool LogTable::IsEmpty() const {
  return Size() == 0;
}

LogTable LogTable::GetSubtable(std::string_view prefix) const {
  // Create a new LogTable with combined prefix
  std::string newPrefix = m_prefix.empty()
    ? NormalizePrefix(prefix)
    : m_prefix + NormalizePrefix(prefix);

  LogTable subtable;
  subtable.m_prefix = newPrefix;
  subtable.m_entries = m_entries;  // Share the same storage
  subtable.m_mutex = m_mutex;      // Share the same mutex

  return subtable;
}

std::unordered_map<std::string, LogValue> LogTable::GetAllEntries() const {
  std::shared_lock lock(*m_mutex);

  // If we have a prefix, filter entries
  if (!m_prefix.empty()) {
    std::unordered_map<std::string, LogValue> filtered;
    for (const auto& [key, value] : *m_entries) {
      if (key.starts_with(m_prefix)) {
        filtered[key] = value;
      }
    }
    return filtered;
  }

  // No prefix, return everything
  return *m_entries;
}

std::vector<std::string> LogTable::GetKeys() const {
  std::shared_lock lock(*m_mutex);

  std::vector<std::string> keys;
  keys.reserve(m_entries->size());

  for (const auto& [key, value] : *m_entries) {
    if (m_prefix.empty() || key.starts_with(m_prefix)) {
      keys.push_back(key);
    }
  }

  return keys;
}

bool LogTable::HasChanged(std::string_view key, const LogValue& newValue) const {
  std::string fullKey = BuildKey(key);
  std::shared_lock lock(*m_mutex);

  auto it = m_entries->find(fullKey);
  if (it == m_entries->end()) {
    return true;  // Key doesn't exist, so it's a change
  }

  return it->second != newValue;
}

std::string LogTable::BuildKey(std::string_view key) const {
  // If key already starts with "/", don't add another one
  if (key.starts_with("/")) {
    return m_prefix + std::string(key);
  }

  // Otherwise, ensure proper "/" separator
  if (m_prefix.empty()) {
    return "/" + std::string(key);
  }

  return m_prefix + "/" + std::string(key);
}

std::string LogTable::NormalizePrefix(std::string_view prefix) {
  if (prefix.empty()) {
    return "";
  }

  std::string normalized(prefix);

  // Ensure it starts with "/"
  if (!normalized.starts_with("/")) {
    normalized = "/" + normalized;
  }

  // Remove trailing "/" if present
  if (normalized.ends_with("/") && normalized.length() > 1) {
    normalized.pop_back();
  }

  return normalized;
}

}  // namespace telemetrykit

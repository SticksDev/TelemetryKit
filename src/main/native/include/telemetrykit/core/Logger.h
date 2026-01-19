#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include "telemetrykit/core/LogTable.h"
#include "telemetrykit/core/LogValue.h"

namespace tkit {

// Forward declaration
class LogDataReceiver;
class LoggableInputs;

/**
 * Global logger singleton.
 *
 * Owns the root LogTable and fans out data to registered receivers
 * (files, NetworkTables, etc.).
 */
class Logger {
 public:
  /// Access the global logger instance.
  static Logger& GetInstance();

  // Lifecycle -------------------------------------------------------------

  /**
   * Starts logging and initializes all receivers.
   * Safe to call more than once.
   */
  void Start();

  /**
   * Pushes the current log table to all receivers.
   * Call once per periodic loop.
   */
  void Periodic();

  /**
   * Stops logging and shuts down receivers.
   */
  void End();

  // Recording -------------------------------------------------------------

  /**
   * Record a value at the given key.
   *
   * This is the common path for most logging calls.
   */
  template<typename T>
  void RecordOutput(std::string_view key, const T& value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_rootTable.Put(key, LogValue(value));
  }

  /**
   * Record a value with unit metadata.
   *
   * Units are used by tools like AdvantageScope for plotting.
   */
  template<typename T>
  void RecordOutput(std::string_view key, const T& value, std::string_view unit) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_rootTable.Put(key, LogValue(value), unit);
  }

  /// Explicit LogValue overloads (used internally and by helpers).
  void RecordOutput(std::string_view key, const LogValue& value);
  void RecordOutput(std::string_view key, const LogValue& value, std::string_view unit);

  /**
   * Input logging hook (AdvantageKit-style).
   *
   * Currently just records the value, but exists to support
   * future log replay or simulation overrides.
   */
  template<typename T>
  void ProcessInput(std::string_view key, const T& value) {
    RecordOutput(key, value);
  }

  /**
   * Process a LoggableInputs object under a prefix.
   */
  void ProcessInputs(std::string_view key, const LoggableInputs& inputs);

  // Table access ----------------------------------------------------------

  /**
   * Returns a prefixed view into the root log table.
   *
   * Useful for grouping related values.
   */
  LogTable GetTable(std::string_view prefix);

  /**
   * Direct access to the root table.
   * Intended for advanced or internal use.
   */
  LogTable& GetRootTable() { return m_rootTable; }

  // Receiver management ---------------------------------------------------

  /**
   * Adds a receiver that will be updated every Periodic() call.
   */
  void AddReceiver(std::unique_ptr<LogDataReceiver> receiver);

  /**
   * Removes all registered receivers.
   */
  void RemoveAllReceivers();

  // State -----------------------------------------------------------------

  /// True while logging is active.
  bool IsLogging() const { return m_isLogging; }

  /**
   * Returns a timestamp in microseconds.
   *
   * Uses FPGA time for consistency with WPILib logs.
   */
  static int64_t GetTimestampUs();

 private:
  Logger() = default;
  ~Logger() = default;
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  /// Checks if Periodic() hasn't been called recently and warns.
  void CheckPeriodicWarning();

  LogTable m_rootTable;
  std::vector<std::unique_ptr<LogDataReceiver>> m_receivers;
  bool m_isLogging{false};
  mutable std::mutex m_mutex;

  // Warning tracking
  int64_t m_lastPeriodicTime{0};
  int64_t m_lastWarningTime{0};
  static constexpr int64_t kPeriodicWarningIntervalUs = 5'000'000;  // 5 seconds
};

// Free-function wrappers --------------------------------------------------

/**
 * Convenience wrapper around Logger::RecordOutput().
 */
template<typename T>
inline void RecordOutput(std::string_view key, const T& value) {
  Logger::GetInstance().RecordOutput(key, value);
}

/**
 * Convenience wrapper around Logger::RecordOutput() with units.
 */
template<typename T>
inline void RecordOutput(std::string_view key, const T& value, std::string_view unit) {
  Logger::GetInstance().RecordOutput(key, value, unit);
}

/**
 * Convenience wrapper around Logger::ProcessInput().
 */
template<typename T>
inline void ProcessInput(std::string_view key, const T& value) {
  Logger::GetInstance().ProcessInput(key, value);
}

/**
 * Forwards to Logger::Periodic().
 */
inline void Periodic() {
  Logger::GetInstance().Periodic();
}

} 

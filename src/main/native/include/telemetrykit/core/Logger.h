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
 * Logger - Main singleton logger for TelemetryKit.
 *
 * The Logger is the central hub for recording telemetry data. It manages
 * the log table and coordinates with receivers (file writers, NetworkTables
 * publishers, etc.).
 */
class Logger {
 public:
  /**
   * Get the singleton Logger instance.
   */
  static Logger& GetInstance();

  // Lifecycle methods

  /**
   * Start logging.
   *
   * Initializes all receivers and prepares for recording.
   * Safe to call multiple times (idempotent).
   */
  void Start();

  /**
   * Update the logger.
   *
   * Call this once per periodic cycle to send logged data to all receivers.
   * This is when data is written to files and published to NetworkTables.
   */
  void Periodic();

  /**
   * End logging.
   *
   * Closes all receivers and cleans up resources.
   */
  void End();

  // Recording API

  /**
   * Record an output value with template type deduction.
   *
   * This is the primary API for logging data.
   *
   * Examples:
   *   logger.RecordOutput("/Speed", 3.5);
   *   logger.RecordOutput("/Enabled", true);
   *   logger.RecordOutput("/Pose", pose2d);
   */
  template<typename T>
  void RecordOutput(std::string_view key, const T& value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_rootTable.Put(key, LogValue(value));
  }

  /**
   * Record an output value with unit metadata.
   *
   * Unit metadata is used by AdvantageScope for unit-aware graphing.
   *
   * Examples:
   *   logger.RecordOutput("/Speed", 3.5, "m/s");
   *   logger.RecordOutput("/Angle", 1.57, "radians");
   *   logger.RecordOutput("/Current", 42.0, "amps");
   */
  template<typename T>
  void RecordOutput(std::string_view key, const T& value, std::string_view unit) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_rootTable.Put(key, LogValue(value), unit);
  }

  /**
   * Record an output value (LogValue overload).
   */
  void RecordOutput(std::string_view key, const LogValue& value);

  /**
   * Record an output value with unit metadata (LogValue overload).
   */
  void RecordOutput(std::string_view key, const LogValue& value, std::string_view unit);


  /**
   * Process input data (for AdvantageKit-style IO pattern).
   *
   * Currently just logs the inputs. In the future, this is where
   * replay data would override real hardware inputs.
   */
  template<typename T>
  void ProcessInput(std::string_view key, const T& value) {
    RecordOutput(key, value);
  }

  /**
   * Process inputs using LoggableInputs interface.
   *
   * Example:
   *   m_gyro->UpdateInputs(m_gyroInputs);
   *   logger.ProcessInputs("/Gyro", m_gyroInputs);
   */
  void ProcessInputs(std::string_view key, const LoggableInputs& inputs);

  // Table access

  /**
   * Get a subtable for organized logging.
   *
   * Example:
   *   auto gyroTable = logger.GetTable("/Gyro");
   *   gyroTable.Put("Yaw", 45.0);
   *   gyroTable.Put("Pitch", 10.0);
   */
  LogTable GetTable(std::string_view prefix);

  /**
   * Get the root log table (for advanced usage).
   */
  LogTable& GetRootTable() { return m_rootTable; }

  // Receiver management

  /**
   * Add a data receiver (e.g., file writer, NetworkTables publisher).
   *
   * Receivers will be notified on each Periodic() call.
   *
   * Example:
   *   logger.AddReceiver(std::make_unique<WPILogWriter>("/logs"));
   */
  void AddReceiver(std::unique_ptr<LogDataReceiver> receiver);

  /**
   * Remove all receivers.
   */
  void RemoveAllReceivers();

  // State

  /**
   * Check if logging is currently active.
   */
  bool IsLogging() const { return m_isLogging; }

  /**
   * Get the current timestamp in microseconds.
   *
   * Uses FPGA timestamp for consistency with WPILib.
   */
  static int64_t GetTimestampUs();

 private:
  // Singleton - private constructor
  Logger() = default;
  ~Logger() = default;
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  LogTable m_rootTable;
  std::vector<std::unique_ptr<LogDataReceiver>> m_receivers;
  bool m_isLogging{false};
  mutable std::mutex m_mutex;
};

// Convenience functions for global access

/**
 * Record an output value (global convenience function).
 *
 * Example:
 *   tkit::RecordOutput("/Speed", 3.5);
 */
template<typename T>
inline void RecordOutput(std::string_view key, const T& value) {
  Logger::GetInstance().RecordOutput(key, value);
}

/**
 * Record an output value with unit metadata (global convenience function).
 *
 * Example:
 *   tkit::RecordOutput("/Speed", 3.5, "m/s");
 *   tkit::RecordOutput("/Angle", 1.57, "radians");
 */
template<typename T>
inline void RecordOutput(std::string_view key, const T& value, std::string_view unit) {
  Logger::GetInstance().RecordOutput(key, value, unit);
}

/**
 * Process input value (global convenience function).
 */
template<typename T>
inline void ProcessInput(std::string_view key, const T& value) {
  Logger::GetInstance().ProcessInput(key, value);
}

/**
 * Update the logger (global convenience function).
 *
 * Sends logged data to all receivers.
 */
inline void Periodic() {
  Logger::GetInstance().Periodic();
}

}  // namespace tkit

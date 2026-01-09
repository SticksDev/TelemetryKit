#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include "telemetrykit/LogTable.h"
#include "telemetrykit/LogValue.h"

namespace telemetrykit {

// Forward declaration
class LogDataReceiver;
class LoggableInputs;

/**
 * Logger - Main singleton logger for TelemetryKit.
 *
 * The Logger is the central hub for recording telemetry data. It manages
 * the log table and coordinates with receivers (file writers, NetworkTables
 * publishers, etc.).
 *
 * Lifecycle:
 *   1. Start() - Initialize logging and receivers
 *   2. PeriodicBeforeUser() - Call before user code each cycle
 *   3. User code runs and calls RecordOutput()
 *   4. PeriodicAfterUser() - Send data to receivers
 *   5. End() - Cleanup and close files
 *
 * Example Usage:
 *
 *   void RobotInit() {
 *     auto& logger = Logger::GetInstance();
 *     logger.Start();
 *
 *     logger.AddReceiver(
 *       std::make_unique<WPILogWriter>("/home/lvuser/logs")
 *     );
 *     logger.AddReceiver(
 *       std::make_unique<NT4Publisher>()
 *     );
 *   }
 *
 *   void RobotPeriodic() {
 *     auto& logger = Logger::GetInstance();
 *     logger.PeriodicBeforeUser();
 *
 *     // Update inputs (IO pattern)
 *     m_gyro->UpdateInputs(m_gyroInputs);
 *     auto gyroTable = logger.GetTable("/Gyro");
 *     m_gyroInputs.ToLog(gyroTable);
 *
 *     // Log outputs
 *     logger.RecordOutput("/Drivetrain/Pose", m_drivetrain.GetPose());
 *     logger.RecordOutput("/Drivetrain/Speed", m_drivetrain.GetSpeed());
 *
 *     logger.PeriodicAfterUser();
 *   }
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
   * Called before user code each periodic cycle.
   *
   * Use this for any pre-cycle initialization.
   * In the future, this is where replay data would be injected.
   */
  void PeriodicBeforeUser();

  /**
   * Called after user code each periodic cycle.
   *
   * Sends the current log table to all receivers.
   * This is when data is written to files and published to NetworkTables.
   */
  void PeriodicAfterUser();

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
   * Record an output value (LogValue overload).
   */
  void RecordOutput(std::string_view key, const LogValue& value);

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
   * Receivers will be notified on each PeriodicAfterUser() call.
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
 *   telemetrykit::RecordOutput("/Speed", 3.5);
 */
template<typename T>
inline void RecordOutput(std::string_view key, const T& value) {
  Logger::GetInstance().RecordOutput(key, value);
}

/**
 * Process input value (global convenience function).
 */
template<typename T>
inline void ProcessInput(std::string_view key, const T& value) {
  Logger::GetInstance().ProcessInput(key, value);
}

}  // namespace telemetrykit

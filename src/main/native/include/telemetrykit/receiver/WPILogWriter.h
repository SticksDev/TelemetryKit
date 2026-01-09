#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <wpi/DataLogWriter.h>

#include "telemetrykit/receiver/LogDataReceiver.h"
#include "telemetrykit/core/LogValue.h"

namespace telemetrykit {

/**
 * WPILogWriter - Writes telemetry data to WPILOG files.
 *
 * Uses WPILib's DataLog format for file output. Files are compatible with
 * AdvantageScope, Glass, and other WPILib visualization tools.
 *
 * This supports all LogValue types, as well as to thread-safe logging via LogTable.
 *
 * Example Usage:
 *
 *   auto& logger = Logger::GetInstance();
 *   logger.AddReceiver(
 *     std::make_unique<WPILogWriter>("/home/lvuser/logs")
 *   );
 */
class WPILogWriter : public LogDataReceiver {
 public:
  /**
   * Create a WPILogWriter that writes to the specified directory.
   *
   * @param logPath Directory path where log files will be created
   *                (default: "/home/lvuser/logs" for RoboRIO)
   */
  explicit WPILogWriter(std::string_view logPath = "/home/lvuser/logs");

  // LogDataReceiver interface
  void OnStart() override;
  void OnUpdate(const LogTable& table, int64_t timestamp) override;
  void OnEnd() override;

 private:
  /**
   * Get or create a DataLog entry for the given key and value type.
   *
   * @param key The log key (e.g., "/Drivetrain/Speed")
   * @param value The value to log
   * @return Entry ID for appending data
   */
  int GetOrCreateEntry(std::string_view key, const LogValue& value);

  /**
   * Append a value to the DataLog.
   *
   * @param entryId The entry ID
   * @param value The value to append
   * @param timestamp Timestamp in microseconds
   */
  void AppendValue(int entryId, const LogValue& value, int64_t timestamp);

  /**
   * Generate a log file name with timestamp.
   */
  std::string GenerateLogFileName() const;

  std::string m_logPath;
  std::unique_ptr<wpi::log::DataLogWriter> m_log;
  std::unordered_map<std::string, int> m_entryIds;  // key -> entry ID
  std::unordered_map<std::string, LogValue> m_lastValues;  // For change detection
};

}  // namespace telemetrykit

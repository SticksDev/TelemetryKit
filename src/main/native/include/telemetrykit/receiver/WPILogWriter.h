#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <wpi/DataLogWriter.h>

#include "telemetrykit/receiver/LogDataReceiver.h"
#include "telemetrykit/core/LogValue.h"

namespace tkit {

/**
 * Receiver that writes logged telemetry to a WPILib DataLog (.wpilog).
 *
 * Output files can be opened in AdvantageScope, Glass, and other WPILib tools.
 * All LogValue types are supported.
 */
class WPILogWriter : public LogDataReceiver {
 public:
  /**
   * Writes log files to the given directory.
   *
   * Defaults to the standard RoboRIO log path.
   */
  explicit WPILogWriter(std::string_view logPath = "/home/lvuser/logs");

  // LogDataReceiver
  void OnStart() override;
  void OnUpdate(const LogTable& table, int64_t timestamp) override;
  void OnEnd() override;

 private:
  /**
   * Returns an existing DataLog entry or creates a new one if needed.
   */
  int GetOrCreateEntry(
      std::string_view key,
      const LogValue& value,
      std::string_view unit = "");

  /**
   * Appends a single value to the log.
   */
  void AppendValue(int entryId, const LogValue& value, int64_t timestamp);

  /// Builds a log file name based on the current time.
  std::string GenerateLogFileName() const;

  std::string m_logPath;
  std::unique_ptr<wpi::log::DataLogWriter> m_log;

  // Cache of log entry IDs by key.
  std::unordered_map<std::string, int> m_entryIds;

  // Last written values, used to skip unchanged data.
  std::unordered_map<std::string, LogValue> m_lastValues;
};

} 

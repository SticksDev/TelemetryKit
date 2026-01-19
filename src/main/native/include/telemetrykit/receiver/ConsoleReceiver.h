#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "telemetrykit/receiver/LogDataReceiver.h"
#include "telemetrykit/core/LogValue.h"

namespace tkit {

/**
 * Receiver that prints log data to the console.
 *
 * Intended for debugging and local development.
 */
class ConsoleReceiver : public LogDataReceiver {
 public:
  /**
   * @param prefix String printed at the start of each line.
   * @param printOnlyChanges If true, unchanged values are skipped.
   */
  explicit ConsoleReceiver(
      std::string_view prefix = "[TelemetryKit]",
      bool printOnlyChanges = true);

  // LogDataReceiver
  void OnStart() override;
  void OnUpdate(const LogTable& table, int64_t timestamp) override;
  void OnEnd() override;

 private:
  /// Converts a LogValue to a printable string.
  std::string FormatValue(const LogValue& value) const;

  std::string m_prefix;
  bool m_printOnlyChanges;
  std::unordered_map<std::string, LogValue> m_lastValues;
  std::ostream& m_output;
};

} 

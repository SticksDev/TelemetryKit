#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "telemetrykit/receiver/LogDataReceiver.h"
#include "telemetrykit/core/LogValue.h"

namespace tkit {

/**
 * ConsoleReceiver - Prints telemetry data to the console for debugging.
 *
 * Useful for development and debugging. Prints all logged values to stdout
 * in a human-readable format.
 *
 * Example Usage:
 *
 *   auto& logger = Logger::GetInstance();
 *   logger.AddReceiver(
 *     std::make_unique<ConsoleReceiver>()
 *   );
 */
class ConsoleReceiver : public LogDataReceiver {
 public:
  /**
   * Create a ConsoleReceiver that prints to stdout.
   *
   * @param prefix Optional prefix for all log lines (default: "[TelemetryKit]")
   * @param printOnlyChanges If true, only print values that changed (default: true)
   */
  explicit ConsoleReceiver(
      std::string_view prefix = "[TelemetryKit]",
      bool printOnlyChanges = true);

  // LogDataReceiver interface
  void OnStart() override;
  void OnUpdate(const LogTable& table, int64_t timestamp) override;
  void OnEnd() override;

 private:
  /**
   * Format a value as a string for printing.
   *
   * @param value The value to format
   * @return String representation
   */
  std::string FormatValue(const LogValue& value) const;

  std::string m_prefix;
  bool m_printOnlyChanges;
  std::unordered_map<std::string, LogValue> m_lastValues;
  std::ostream& m_output;
};

}  // namespace tkit

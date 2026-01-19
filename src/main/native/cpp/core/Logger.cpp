#include "telemetrykit/core/Logger.h"

#include <frc/Errors.h>
#include <frc/RobotController.h>

#include "telemetrykit/core/AlertManager.h"
#include "telemetrykit/core/LoggableInputs.h"
#include "telemetrykit/receiver/LogDataReceiver.h"

namespace tkit {

// Returns the current logger instance as a singleton (thread-safe). 
// If the instance does not exist, it is created.
Logger& Logger::GetInstance() {
  static Logger instance;
  return instance;
}

// Starts the logging process. 
// Initializes all registered log data receivers and then sets the logging state to active.
void Logger::Start() {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_isLogging) {
    return;  // Already started
  }

  // If no receivers are registered, do nothing
  if (m_receivers.empty()) {
    FRC_ReportError(frc::err::Error, "{}",
        ">>> TelemetryKit Could NOT Initialize!!! <<<\n"
        "No log data receivers are registered. Please add at least one receiver "
        "before starting the logger (e.g., NetworkTablesReceiver or WPILogWriter)."
        "Because of this, TelemetryKit logging will be disabled.\n"
        ">>> TelemetryKit Could NOT Initialize!!! <<<");
    return;
  }

  // Initialize all receivers
  for (auto& receiver : m_receivers) {
    receiver->OnStart();
  }

  m_isLogging = true;
}

void Logger::Periodic() {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_isLogging) {
    return;
  }

  // Get current timestamp
  int64_t timestamp = GetTimestampUs();

  // Update last periodic call time
  m_lastPeriodicTime = timestamp;

  // Send log table to all receivers
  for (auto& receiver : m_receivers) {
    receiver->OnUpdate(m_rootTable, timestamp);
  }

  // Check auto-monitored alerts
  AlertManager::GetInstance().CheckAll();
}

void Logger::CheckPeriodicWarning() {
  if (!m_isLogging) {
    return;
  }

  int64_t now = GetTimestampUs();

  // Skip if Periodic was never called (logging just started)
  if (m_lastPeriodicTime == 0) {
    m_lastPeriodicTime = now;
    return;
  }

  int64_t timeSinceLastPeriodic = now - m_lastPeriodicTime;

  // Warn if Periodic hasn't been called in 5 seconds
  if (timeSinceLastPeriodic >= kPeriodicWarningIntervalUs) {
    // Only warn once per interval
    if (now - m_lastWarningTime >= kPeriodicWarningIntervalUs) {
      FRC_ReportError(frc::warn::Warning, "{}",
          "TelemetryKit: Logger::Periodic() has not been called in 5+ seconds. "
          "Make sure to call tkit::Periodic() in your robot's Periodic() method.");
      m_lastWarningTime = now;
    }
  }
}

void Logger::End() {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_isLogging) {
    return;  // Already stopped
  }

  // Cleanup all receivers
  for (auto& receiver : m_receivers) {
    receiver->OnEnd();
  }

  m_isLogging = false;
}

void Logger::RecordOutput(std::string_view key, const LogValue& value) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_rootTable.Put(key, value);
  CheckPeriodicWarning();
}

void Logger::RecordOutput(std::string_view key, const LogValue& value, std::string_view unit) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_rootTable.Put(key, value, unit);
  CheckPeriodicWarning();
}

void Logger::ProcessInputs(std::string_view key, const LoggableInputs& inputs) {
  // Get subtable for this input group
  auto table = GetTable(key);

  // Log the inputs
  inputs.ToLog(table);
}

LogTable Logger::GetTable(std::string_view prefix) {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_rootTable.GetSubtable(prefix);
}

void Logger::AddReceiver(std::unique_ptr<LogDataReceiver> receiver) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_receivers.push_back(std::move(receiver));
}

void Logger::RemoveAllReceivers() {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_receivers.clear();
}

int64_t Logger::GetTimestampUs() {
  // Use FPGA timestamp for consistency with WPILib
  return frc::RobotController::GetFPGATime();
}

}  // namespace tkit

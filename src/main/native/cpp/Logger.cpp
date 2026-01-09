#include "telemetrykit/Logger.h"

#include <frc/RobotController.h>

#include "telemetrykit/LogDataReceiver.h"
#include "telemetrykit/LoggableInputs.h"

namespace telemetrykit {

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

  // Initialize all receivers
  for (auto& receiver : m_receivers) {
    receiver->OnStart();
  }

  m_isLogging = true;
}

void Logger::PeriodicBeforeUser() {
  // Currently a placeholder
  // In the future, this is where replay data would be injected
}

void Logger::PeriodicAfterUser() {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_isLogging) {
    return;
  }

  // Get current timestamp
  int64_t timestamp = GetTimestampUs();

  // Send log table to all receivers
  for (auto& receiver : m_receivers) {
    receiver->OnUpdate(m_rootTable, timestamp);
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

}  // namespace telemetrykit

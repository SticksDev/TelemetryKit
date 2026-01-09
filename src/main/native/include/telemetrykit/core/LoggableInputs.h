#pragma once

namespace telemetrykit {

// Forward declaration
class LogTable;

/**
 * LoggableInputs - Base class for IO interface input recording pattern.
 *
 * Implementations define what data flows INTO the robot code from hardware.
 * This is the core of the AdvantageKit input-first philosophy.
 */
class LoggableInputs {
 public:
  virtual ~LoggableInputs() = default;

  /**
   * Serialize this input struct to the log table.
   *
   * Each implementation should log all relevant input fields.
   */
  virtual void ToLog(LogTable& table) const = 0;

  /**
   * Optional: Deserialize from log table (for future replay support).
   *
   * Default implementation does nothing.
   */
  virtual void FromLog(const LogTable& table) {}
};

}  // namespace telemetrykit

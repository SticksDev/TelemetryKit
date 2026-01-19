#pragma once

namespace tkit {

// Forward declaration
class LogTable;

/**
 * Base class for logging hardware/input state.
 *
 * Used by subsystems to expose the data coming *into* robot code
 * (sensor readings, device state, etc.).
 */
class LoggableInputs {
 public:
  virtual ~LoggableInputs() = default;

  /**
   * Writes all relevant fields to the given log table.
   */
  virtual void ToLog(LogTable& table) const = 0;

  /**
   * Reads fields from the log table.
   *
   * Intended for future log replay or simulation support.
   * Default implementation does nothing.
   */
  virtual void FromLog(const LogTable& table) {}
};

} 

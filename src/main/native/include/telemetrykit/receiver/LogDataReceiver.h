#pragma once

#include <cstdint>

namespace tkit {

// Forward declaration
class LogTable;

/**
 * Base class for telemetry log receivers.
 *
 * A receiver consumes the current LogTable and sends data somewhere
 * (file, NetworkTables, console, etc.).
 */
class LogDataReceiver {
 public:
  virtual ~LogDataReceiver() = default;

  /**
   * Called once when logging begins.
   *
   * Use this to allocate or initialize resources.
   */
  virtual void OnStart() = 0;

  /**
   * Called periodically with the current log data.
   *
   * Receivers are responsible for deciding what to publish
   * (including any change detection or filtering).
   *
   * @param table Current log table
   * @param timestamp Timestamp in microseconds
   */
  virtual void OnUpdate(const LogTable& table, int64_t timestamp) = 0;

  /**
   * Called once when logging ends.
   *
   * Clean up any resources here.
   */
  virtual void OnEnd() = 0;
};

} 

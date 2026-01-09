#pragma once

#include <cstdint>

namespace telemetrykit {

// Forward declaration
class LogTable;

/**
 * LogDataReceiver - Abstract base class for data receivers.
 *
 * Receivers process the log table and send data to their destinations.
 * Examples: WPILog file writer, NetworkTables publisher, console logger.
 *
 * Each receiver is responsible for its own change detection and optimization.
 *
 * Example Implementation:
 *
 *   class MyReceiver : public LogDataReceiver {
 *    public:
 *     void OnStart() override {
 *       // Initialize resources (open files, create publishers, etc.)
 *     }
 *
 *     void OnUpdate(const LogTable& table, int64_t timestamp) override {
 *       // Process the log table
 *       // Perform change detection if desired
 *       // Write/publish data
 *     }
 *
 *     void OnEnd() override {
 *       // Cleanup resources (close files, etc.)
 *     }
 *   };
 */
class LogDataReceiver {
 public:
  virtual ~LogDataReceiver() = default;

  /**
   * Called when logging starts.
   *
   * Use this to initialize resources:
   *   - Open log files
   *   - Create NetworkTables publishers
   *   - Allocate buffers
   */
  virtual void OnStart() = 0;

  /**
   * Called each periodic cycle with the current log table.
   *
   * The receiver should process the table and send data to its destination.
   * Change detection is the receiver's responsibility.
   *
   * @param table The current log table with all recorded data
   * @param timestamp Current timestamp in microseconds
   */
  virtual void OnUpdate(const LogTable& table, int64_t timestamp) = 0;

  /**
   * Called when logging ends.
   *
   * Use this to cleanup resources:
   *   - Close log files
   *   - Flush buffers
   *   - Release publishers
   */
  virtual void OnEnd() = 0;
};

}  // namespace telemetrykit

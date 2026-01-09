#pragma once

namespace telemetrykit {

// Forward declaration
class LogTable;

/**
 * LoggableInputs - Base class for IO interface input recording pattern.
 *
 * Implementations define what data flows INTO the robot code from hardware.
 * This is the core of the AdvantageKit input-first philosophy.
 *
 * Example Usage:
 *
 *   // Define inputs struct
 *   struct GyroInputs : LoggableInputs {
 *     double yawDegrees{0.0};
 *     double pitchDegrees{0.0};
 *     bool isConnected{false};
 *
 *     void ToLog(LogTable& table) const override {
 *       table.Put("YawDegrees", yawDegrees);
 *       table.Put("PitchDegrees", pitchDegrees);
 *       table.Put("Connected", isConnected);
 *     }
 *   };
 *
 *   // Hardware abstraction interface
 *   class GyroIO {
 *    public:
 *     virtual void UpdateInputs(GyroInputs& inputs) = 0;
 *   };
 *
 *   // Real hardware implementation
 *   class GyroIOPigeon2 : public GyroIO {
 *    public:
 *     void UpdateInputs(GyroInputs& inputs) override {
 *       inputs.yawDegrees = m_pigeon.GetYaw();
 *       // ... read actual hardware
 *     }
 *   };
 *
 *   // In robot code:
 *   m_gyro->UpdateInputs(m_gyroInputs);  // Read hardware
 *   auto gyroTable = logger.GetTable("/Gyro");
 *   m_gyroInputs.ToLog(gyroTable);       // Log inputs
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

#pragma once

#include <wpi/struct/Struct.h>

#include <frc/geometry/Pose2d.h>
#include <frc/geometry/Pose3d.h>
#include <frc/geometry/Translation2d.h>
#include <frc/geometry/Translation3d.h>
#include <frc/kinematics/SwerveModulePosition.h>
#include <frc/kinematics/SwerveModuleState.h>

#include "telemetrykit/core/LogTable.h"
#include "telemetrykit/core/LogValue.h"

namespace tkit {

/**
 * Helper functions to log WPILib structs with field decomposition.
 *
 * These functions log individual fields only (not the raw struct),
 * making them easily visible and plottable in dashboards.
 *
 * For logging the raw struct, use the regular RecordOutput() method.
 */

/**
 * Log a Pose2d with decomposed fields.
 *
 * Logs:
 *   key/X (double meters)
 *   key/Y (double meters)
 *   key/Rotation (double radians)
 */
inline void LogPose2d(LogTable& table, std::string_view key, const frc::Pose2d& pose) {
  std::string baseKey(key);
  table.Put(baseKey + "/X", pose.X().value());
  table.Put(baseKey + "/Y", pose.Y().value());
  table.Put(baseKey + "/Rotation", pose.Rotation().Radians().value());
}

/**
 * Log a Pose3d with decomposed fields.
 *
 * Logs:
 *   key/X, Y, Z (double meters)
 *   key/Roll, Pitch, Yaw (double radians)
 */
inline void LogPose3d(LogTable& table, std::string_view key, const frc::Pose3d& pose) {
  std::string baseKey(key);
  table.Put(baseKey + "/X", pose.X().value());
  table.Put(baseKey + "/Y", pose.Y().value());
  table.Put(baseKey + "/Z", pose.Z().value());

  auto quaternion = pose.Rotation().GetQuaternion();
  table.Put(baseKey + "/QuaternionW", quaternion.W());
  table.Put(baseKey + "/QuaternionX", quaternion.X());
  table.Put(baseKey + "/QuaternionY", quaternion.Y());
  table.Put(baseKey + "/QuaternionZ", quaternion.Z());
}

/**
 * Log a Translation2d with decomposed fields.
 */
inline void LogTranslation2d(LogTable& table, std::string_view key, const frc::Translation2d& translation) {
  std::string baseKey(key);
  table.Put(baseKey + "/X", translation.X().value());
  table.Put(baseKey + "/Y", translation.Y().value());
}

/**
 * Log a Translation3d with decomposed fields.
 */
inline void LogTranslation3d(LogTable& table, std::string_view key, const frc::Translation3d& translation) {
  std::string baseKey(key);
  table.Put(baseKey + "/X", translation.X().value());
  table.Put(baseKey + "/Y", translation.Y().value());
  table.Put(baseKey + "/Z", translation.Z().value());
}

/**
 * Log a SwerveModuleState with decomposed fields.
 */
inline void LogSwerveModuleState(LogTable& table, std::string_view key, const frc::SwerveModuleState& state) {
  std::string baseKey(key);
  table.Put(baseKey + "/Speed", state.speed.value());
  table.Put(baseKey + "/Angle", state.angle.Radians().value());
}

/**
 * Log a SwerveModulePosition with decomposed fields.
 */
inline void LogSwerveModulePosition(LogTable& table, std::string_view key, const frc::SwerveModulePosition& position) {
  std::string baseKey(key);
  table.Put(baseKey + "/Distance", position.distance.value());
  table.Put(baseKey + "/Angle", position.angle.Radians().value());
}

/**
 * Log an array of SwerveModuleStates with decomposed fields.
 *
 * Example:
 *   LogSwerveModuleStates(table, "/Drive/ModuleStates", states);
 *   // Logs: /Drive/ModuleStates/Module0/Speed
 *   //       /Drive/ModuleStates/Module0/Angle
 *   //       /Drive/ModuleStates/Module1/Speed
 *   //       ... etc
 */
inline void LogSwerveModuleStates(LogTable& table, std::string_view key,
                                   const std::vector<frc::SwerveModuleState>& states) {
  for (size_t i = 0; i < states.size(); ++i) {
    std::string moduleKey = std::string(key) + "/Module" + std::to_string(i);
    LogSwerveModuleState(table, moduleKey, states[i]);
  }
}

/**
 * Log an array of SwerveModulePositions with decomposed fields.
 */
inline void LogSwerveModulePositions(LogTable& table, std::string_view key,
                                      const std::vector<frc::SwerveModulePosition>& positions) {
  for (size_t i = 0; i < positions.size(); ++i) {
    std::string moduleKey = std::string(key) + "/Module" + std::to_string(i);
    LogSwerveModulePosition(table, moduleKey, positions[i]);
  }
}

}  // namespace tkit

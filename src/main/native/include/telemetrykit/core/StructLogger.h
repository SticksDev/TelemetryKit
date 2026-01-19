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
 * Helpers for logging common WPILib structs by decomposing them into fields.
 *
 * Each function expands the struct into individual numeric entries so they
 * can be plotted or inspected easily in dashboards.
 */

inline void LogPose2d(LogTable& table,
                      std::string_view key,
                      const frc::Pose2d& pose) {
  std::string baseKey(key);
  table.Put(baseKey + "/X", pose.X().value());
  table.Put(baseKey + "/Y", pose.Y().value());
  table.Put(baseKey + "/Rotation", pose.Rotation().Radians().value());
}

inline void LogPose3d(LogTable& table,
                      std::string_view key,
                      const frc::Pose3d& pose) {
  std::string baseKey(key);
  table.Put(baseKey + "/X", pose.X().value());
  table.Put(baseKey + "/Y", pose.Y().value());
  table.Put(baseKey + "/Z", pose.Z().value());

  auto q = pose.Rotation().GetQuaternion();
  table.Put(baseKey + "/QuaternionW", q.W());
  table.Put(baseKey + "/QuaternionX", q.X());
  table.Put(baseKey + "/QuaternionY", q.Y());
  table.Put(baseKey + "/QuaternionZ", q.Z());
}

inline void LogTranslation2d(LogTable& table,
                             std::string_view key,
                             const frc::Translation2d& translation) {
  std::string baseKey(key);
  table.Put(baseKey + "/X", translation.X().value());
  table.Put(baseKey + "/Y", translation.Y().value());
}

inline void LogTranslation3d(LogTable& table,
                             std::string_view key,
                             const frc::Translation3d& translation) {
  std::string baseKey(key);
  table.Put(baseKey + "/X", translation.X().value());
  table.Put(baseKey + "/Y", translation.Y().value());
  table.Put(baseKey + "/Z", translation.Z().value());
}

inline void LogSwerveModuleState(LogTable& table,
                                 std::string_view key,
                                 const frc::SwerveModuleState& state) {
  std::string baseKey(key);
  table.Put(baseKey + "/Speed", state.speed.value());
  table.Put(baseKey + "/Angle", state.angle.Radians().value());
}

inline void LogSwerveModulePosition(LogTable& table,
                                    std::string_view key,
                                    const frc::SwerveModulePosition& position) {
  std::string baseKey(key);
  table.Put(baseKey + "/Distance", position.distance.value());
  table.Put(baseKey + "/Angle", position.angle.Radians().value());
}

inline void LogSwerveModuleStates(
    LogTable& table,
    std::string_view key,
    const std::vector<frc::SwerveModuleState>& states) {
  for (size_t i = 0; i < states.size(); ++i) {
    LogSwerveModuleState(
        table,
        std::string(key) + "/Module" + std::to_string(i),
        states[i]);
  }
}

inline void LogSwerveModulePositions(
    LogTable& table,
    std::string_view key,
    const std::vector<frc::SwerveModulePosition>& positions) {
  for (size_t i = 0; i < positions.size(); ++i) {
    LogSwerveModulePosition(
        table,
        std::string(key) + "/Module" + std::to_string(i),
        positions[i]);
  }
}

} 

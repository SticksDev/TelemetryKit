---
icon: lucide/code
---

# Basic Examples

This page provides practical examples of using TelemetryKit in common FRC robot scenarios.

## Minimal Setup

The simplest possible TelemetryKit setup:

```cpp title="Robot.cpp"
#include <frc/TimedRobot.h>
#include <telemetrykit/TelemetryKit.h>

class Robot : public frc::TimedRobot {
 public:
  void RobotInit() override {
    auto& logger = tkit::Logger::GetInstance();
    logger.AddReceiver(std::make_unique<tkit::NetworkTablesReceiver>());
    logger.Start();
  }

  void RobotPeriodic() override {
    tkit::Logger::RecordOutput("Timestamp", tkit::Logger::GetTimestampUs());
    tkit::Logger::Periodic();
  }
};
```

## Drivetrain Telemetry

Log motor speeds, encoder positions, and gyro angle:

```cpp title="Drivetrain.h"
#include <frc/motorcontrol/PWMSparkMax.h>
#include <frc/Encoder.h>
#include <AHRS.h>
#include <telemetrykit/TelemetryKit.h>

class Drivetrain {
 public:
  void UpdateTelemetry() {
    // Motor speeds
    tkit::Logger::RecordOutput("Drive/LeftSpeed", m_leftMotor.Get());
    tkit::Logger::RecordOutput("Drive/RightSpeed", m_rightMotor.Get());

    // Encoder positions
    tkit::Logger::RecordOutput("Drive/LeftPosition", m_leftEncoder.GetDistance());
    tkit::Logger::RecordOutput("Drive/RightPosition", m_rightEncoder.GetDistance());

    // Gyro angle
    tkit::Logger::RecordOutput("Drive/GyroAngle", m_gyro.GetAngle());

    // Battery voltage
    tkit::Logger::RecordOutput("Drive/BatteryVoltage",
      frc::RobotController::GetBatteryVoltage());
  }

 private:
  frc::PWMSparkMax m_leftMotor{0};
  frc::PWMSparkMax m_rightMotor{1};
  frc::Encoder m_leftEncoder{0, 1};
  frc::Encoder m_rightEncoder{2, 3};
  AHRS m_gyro{frc::SPI::kMXP};
};
```

## Swerve Drive with Odometry

Complete swerve drive telemetry including pose estimation:

```cpp title="SwerveDrive.cpp"
#include <frc/geometry/Pose2d.h>
#include <frc/kinematics/SwerveDriveOdometry.h>
#include <telemetrykit/TelemetryKit.h>
#include <telemetrykit/core/StructLogger.h>

class SwerveDrive {
 public:
  void UpdateTelemetry() {
    auto& table = tkit::Logger::GetTable();

    // Robot pose (multiple formats for different uses)
    frc::Pose2d pose = m_odometry.GetPose();

    // Full struct for AdvantageScope visualization
    tkit::Logger::RecordOutput("Swerve/Pose", tkit::MakeStructValue(pose));

    // Decomposed for dashboard graphing
    tkit::LogPose2d(table, "Swerve/PoseFields", pose);

    // Module states (decomposed)
    tkit::LogSwerveModuleStates(table, "Swerve/MeasuredStates", m_measuredStates);
    tkit::LogSwerveModuleStates(table, "Swerve/DesiredStates", m_desiredStates);

    // Module states as struct arrays for AdvantageScope
    tkit::Logger::RecordOutput("Swerve/MeasuredStatesStruct",
      tkit::MakeStructArrayValue(std::span(m_measuredStates)));
    tkit::Logger::RecordOutput("Swerve/DesiredStatesStruct",
      tkit::MakeStructArrayValue(std::span(m_desiredStates)));

    // Gyro angle
    tkit::Logger::RecordOutput("Swerve/GyroAngle", m_gyro.GetRotation2d().Degrees().value());
  }

 private:
  frc::SwerveDriveOdometry<4> m_odometry;
  std::array<frc::SwerveModuleState, 4> m_measuredStates;
  std::array<frc::SwerveModuleState, 4> m_desiredStates;
  AHRS m_gyro{frc::SPI::kMXP};
};
```

## Vision System

Log AprilTag detections and pose estimates:

```cpp title="VisionSystem.cpp"
#include <photon/PhotonCamera.h>
#include <telemetrykit/TelemetryKit.h>

class VisionSystem {
 public:
  void UpdateTelemetry() {
    auto result = m_camera.GetLatestResult();

    // Detection info
    tkit::Logger::RecordOutput("Vision/HasTargets", result.HasTargets());
    tkit::Logger::RecordOutput("Vision/TargetCount", static_cast<int64_t>(result.GetTargets().size()));
    tkit::Logger::RecordOutput("Vision/Latency", result.GetLatency());

    if (result.HasTargets()) {
      auto target = result.GetBestTarget();

      // Target data
      tkit::Logger::RecordOutput("Vision/BestTarget/ID", target.GetFiducialId());
      tkit::Logger::RecordOutput("Vision/BestTarget/Yaw", target.GetYaw());
      tkit::Logger::RecordOutput("Vision/BestTarget/Pitch", target.GetPitch());
      tkit::Logger::RecordOutput("Vision/BestTarget/Area", target.GetArea());

      // Robot pose from vision
      auto poseEstimate = m_poseEstimator.Update(result);
      if (poseEstimate.has_value()) {
        tkit::Logger::RecordOutput("Vision/EstimatedPose",
          tkit::MakeStructValue(poseEstimate.value().estimatedPose.ToPose2d()));
      }
    }
  }

 private:
  photon::PhotonCamera m_camera{"PhotonVision"};
  photon::PhotonPoseEstimator m_poseEstimator;
};
```

## Subsystem IO Pattern

AdvantageKit-style IO pattern with TelemetryKit:

```cpp title="ElevatorIO.h"
#include <telemetrykit/core/LoggableInputs.h>

struct ElevatorIOInputs : public tkit::LoggableInputs {
  double positionMeters = 0.0;
  double velocityMetersPerSecond = 0.0;
  double appliedVolts = 0.0;
  double currentAmps = 0.0;

  void ToLog(tkit::LogTable& table, std::string_view prefix) override {
    std::string p(prefix);
    table.Put(p + "PositionMeters", positionMeters);
    table.Put(p + "VelocityMetersPerSecond", velocityMetersPerSecond);
    table.Put(p + "AppliedVolts", appliedVolts);
    table.Put(p + "CurrentAmps", currentAmps);
  }
};

class ElevatorIO {
 public:
  virtual void UpdateInputs(ElevatorIOInputs& inputs) = 0;
  virtual void SetVoltage(double volts) = 0;
};
```

```cpp title="Elevator.cpp"
#include "ElevatorIO.h"
#include <telemetrykit/TelemetryKit.h>

class Elevator {
 public:
  void Periodic() {
    // Update inputs
    m_io->UpdateInputs(m_inputs);

    // Log to TelemetryKit
    tkit::Logger::ProcessInputs("Elevator", m_inputs);

    // Control logic...
  }

 private:
  std::unique_ptr<ElevatorIO> m_io;
  ElevatorIOInputs m_inputs;
};
```

## Auto Routine Tracking

Track which auto routine is selected and running:

```cpp title="Robot.cpp"
class Robot : public frc::TimedRobot {
 public:
  void AutonomousInit() override {
    std::string selectedAuto = m_chooser.GetSelected();
    tkit::Logger::RecordOutput("Auto/Selected", selectedAuto);
    tkit::Logger::RecordOutput("Auto/StartTime", tkit::Logger::GetTimestampUs());

    m_autoCommand = GetAutoCommand(selectedAuto);
    m_autoCommand->Schedule();
  }

  void AutonomousPeriodic() override {
    // Log command states
    auto scheduler = frc2::CommandScheduler::GetInstance();
    auto runningCommands = GetRunningCommandNames(scheduler);

    tkit::Logger::RecordOutput("Auto/RunningCommands", runningCommands);
    tkit::Logger::Periodic();
  }

 private:
  frc::SendableChooser<std::string> m_chooser;
  frc2::CommandPtr m_autoCommand;
};
```

## Diagnostics and Health Monitoring

Monitor robot health metrics:

```cpp title="Diagnostics.cpp"
#include <frc/RobotController.h>
#include <telemetrykit/TelemetryKit.h>

class Diagnostics {
 public:
  void UpdateTelemetry() {
    // Power
    tkit::Logger::RecordOutput("Diagnostics/BatteryVoltage",
      frc::RobotController::GetBatteryVoltage());
    tkit::Logger::RecordOutput("Diagnostics/3v3Rail",
      frc::RobotController::GetVoltage3V3());
    tkit::Logger::RecordOutput("Diagnostics/5vRail",
      frc::RobotController::GetVoltage5V());
    tkit::Logger::RecordOutput("Diagnostics/6vRail",
      frc::RobotController::GetVoltage6V());

    // FMS
    tkit::Logger::RecordOutput("Diagnostics/FMSAttached",
      frc::DriverStation::IsFMSAttached());
    tkit::Logger::RecordOutput("Diagnostics/MatchTime",
      frc::DriverStation::GetMatchTime());
  }

 private:
  double GetFreeRAM();
  double GetCPUTemperature();
};
```

[Advanced Topics →](../advanced/structs.md){ .md-button .md-button--primary }

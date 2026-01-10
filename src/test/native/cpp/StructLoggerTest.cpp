#include <gtest/gtest.h>

#include <frc/geometry/Pose2d.h>
#include <frc/geometry/Pose3d.h>
#include <frc/geometry/Rotation2d.h>
#include <frc/geometry/Rotation3d.h>
#include <frc/geometry/Translation2d.h>
#include <frc/geometry/Translation3d.h>
#include <frc/kinematics/SwerveModulePosition.h>
#include <frc/kinematics/SwerveModuleState.h>

#include "telemetrykit/core/StructLogger.h"
#include "telemetrykit/core/LogTable.h"

using namespace tkit;
using namespace units::literals;

// Test Pose2d decomposition
TEST(StructLoggerTest, LogPose2d) {
  LogTable table;
  frc::Pose2d pose{1.5_m, 2.3_m, frc::Rotation2d(45_deg)};

  LogPose2d(table, "/RobotPose", pose);

  // Verify all fields were logged
  ASSERT_TRUE(table.Contains("/RobotPose/X"));
  ASSERT_TRUE(table.Contains("/RobotPose/Y"));
  ASSERT_TRUE(table.Contains("/RobotPose/Rotation"));

  // Verify values
  auto x = table.Get("/RobotPose/X");
  auto y = table.Get("/RobotPose/Y");
  auto rotation = table.Get("/RobotPose/Rotation");

  ASSERT_TRUE(x.has_value());
  ASSERT_TRUE(y.has_value());
  ASSERT_TRUE(rotation.has_value());

  EXPECT_DOUBLE_EQ(x->Get<double>(), 1.5);
  EXPECT_DOUBLE_EQ(y->Get<double>(), 2.3);
  EXPECT_NEAR(rotation->Get<double>(), 0.7853981633974483, 1e-9);  // 45 degrees in radians
}

// Test Pose3d decomposition
TEST(StructLoggerTest, LogPose3d) {
  LogTable table;
  frc::Rotation3d rotation{0.1_rad, 0.2_rad, 0.3_rad};
  frc::Pose3d pose{1.0_m, 2.0_m, 3.0_m, rotation};

  LogPose3d(table, "/RobotPose3d", pose);

  // Verify position fields
  ASSERT_TRUE(table.Contains("/RobotPose3d/X"));
  ASSERT_TRUE(table.Contains("/RobotPose3d/Y"));
  ASSERT_TRUE(table.Contains("/RobotPose3d/Z"));

  // Verify quaternion fields
  ASSERT_TRUE(table.Contains("/RobotPose3d/QuaternionW"));
  ASSERT_TRUE(table.Contains("/RobotPose3d/QuaternionX"));
  ASSERT_TRUE(table.Contains("/RobotPose3d/QuaternionY"));
  ASSERT_TRUE(table.Contains("/RobotPose3d/QuaternionZ"));

  // Verify position values
  EXPECT_DOUBLE_EQ(table.Get("/RobotPose3d/X")->Get<double>(), 1.0);
  EXPECT_DOUBLE_EQ(table.Get("/RobotPose3d/Y")->Get<double>(), 2.0);
  EXPECT_DOUBLE_EQ(table.Get("/RobotPose3d/Z")->Get<double>(), 3.0);

  // Verify quaternion values exist (specific values depend on rotation)
  auto quatW = table.Get("/RobotPose3d/QuaternionW");
  auto quatX = table.Get("/RobotPose3d/QuaternionX");
  auto quatY = table.Get("/RobotPose3d/QuaternionY");
  auto quatZ = table.Get("/RobotPose3d/QuaternionZ");

  ASSERT_TRUE(quatW.has_value());
  ASSERT_TRUE(quatX.has_value());
  ASSERT_TRUE(quatY.has_value());
  ASSERT_TRUE(quatZ.has_value());
}

// Test Translation2d decomposition
TEST(StructLoggerTest, LogTranslation2d) {
  LogTable table;
  frc::Translation2d translation{4.5_m, 6.7_m};

  LogTranslation2d(table, "/Translation", translation);

  ASSERT_TRUE(table.Contains("/Translation/X"));
  ASSERT_TRUE(table.Contains("/Translation/Y"));

  EXPECT_DOUBLE_EQ(table.Get("/Translation/X")->Get<double>(), 4.5);
  EXPECT_DOUBLE_EQ(table.Get("/Translation/Y")->Get<double>(), 6.7);
}

// Test Translation3d decomposition
TEST(StructLoggerTest, LogTranslation3d) {
  LogTable table;
  frc::Translation3d translation{1.1_m, 2.2_m, 3.3_m};

  LogTranslation3d(table, "/Translation3d", translation);

  ASSERT_TRUE(table.Contains("/Translation3d/X"));
  ASSERT_TRUE(table.Contains("/Translation3d/Y"));
  ASSERT_TRUE(table.Contains("/Translation3d/Z"));

  EXPECT_DOUBLE_EQ(table.Get("/Translation3d/X")->Get<double>(), 1.1);
  EXPECT_DOUBLE_EQ(table.Get("/Translation3d/Y")->Get<double>(), 2.2);
  EXPECT_DOUBLE_EQ(table.Get("/Translation3d/Z")->Get<double>(), 3.3);
}

// Test SwerveModuleState decomposition
TEST(StructLoggerTest, LogSwerveModuleState) {
  LogTable table;
  frc::SwerveModuleState state{2.5_mps, frc::Rotation2d(30_deg)};

  LogSwerveModuleState(table, "/Module", state);

  ASSERT_TRUE(table.Contains("/Module/Speed"));
  ASSERT_TRUE(table.Contains("/Module/Angle"));

  EXPECT_DOUBLE_EQ(table.Get("/Module/Speed")->Get<double>(), 2.5);
  EXPECT_NEAR(table.Get("/Module/Angle")->Get<double>(), 0.5235987755982988, 1e-9);  // 30 degrees
}

// Test SwerveModulePosition decomposition
TEST(StructLoggerTest, LogSwerveModulePosition) {
  LogTable table;
  frc::SwerveModulePosition position{10.5_m, frc::Rotation2d(60_deg)};

  LogSwerveModulePosition(table, "/Position", position);

  ASSERT_TRUE(table.Contains("/Position/Distance"));
  ASSERT_TRUE(table.Contains("/Position/Angle"));

  EXPECT_DOUBLE_EQ(table.Get("/Position/Distance")->Get<double>(), 10.5);
  EXPECT_NEAR(table.Get("/Position/Angle")->Get<double>(), 1.0471975511965976, 1e-9);  // 60 degrees
}

// Test SwerveModuleStates array decomposition
TEST(StructLoggerTest, LogSwerveModuleStates) {
  LogTable table;
  std::vector<frc::SwerveModuleState> states{
    frc::SwerveModuleState{1.0_mps, frc::Rotation2d(0_deg)},
    frc::SwerveModuleState{2.0_mps, frc::Rotation2d(90_deg)},
    frc::SwerveModuleState{3.0_mps, frc::Rotation2d(180_deg)},
    frc::SwerveModuleState{4.0_mps, frc::Rotation2d(270_deg)}
  };

  LogSwerveModuleStates(table, "/Drive/States", states);

  // Verify all modules were logged
  for (size_t i = 0; i < states.size(); ++i) {
    std::string prefix = "/Drive/States/Module" + std::to_string(i);
    ASSERT_TRUE(table.Contains(prefix + "/Speed")) << "Missing " << prefix + "/Speed";
    ASSERT_TRUE(table.Contains(prefix + "/Angle")) << "Missing " << prefix + "/Angle";

    EXPECT_DOUBLE_EQ(table.Get(prefix + "/Speed")->Get<double>(), static_cast<double>(i + 1));
  }

  // Verify specific angles
  EXPECT_NEAR(table.Get("/Drive/States/Module0/Angle")->Get<double>(), 0.0, 1e-9);
  EXPECT_NEAR(table.Get("/Drive/States/Module1/Angle")->Get<double>(), 1.5707963267948966, 1e-9);  // 90 deg
  EXPECT_NEAR(table.Get("/Drive/States/Module2/Angle")->Get<double>(), 3.141592653589793, 1e-9);   // 180 deg
  // 270 degrees normalizes to 4.712... radians (equivalent to -90 degrees)
  EXPECT_NEAR(table.Get("/Drive/States/Module3/Angle")->Get<double>(), 4.7123889803846897, 1e-9); // 270 deg
}

// Test SwerveModulePositions array decomposition
TEST(StructLoggerTest, LogSwerveModulePositions) {
  LogTable table;
  std::vector<frc::SwerveModulePosition> positions{
    frc::SwerveModulePosition{5.0_m, frc::Rotation2d(0_deg)},
    frc::SwerveModulePosition{10.0_m, frc::Rotation2d(45_deg)},
    frc::SwerveModulePosition{15.0_m, frc::Rotation2d(90_deg)}
  };

  LogSwerveModulePositions(table, "/Drive/Positions", positions);

  // Verify all modules were logged
  for (size_t i = 0; i < positions.size(); ++i) {
    std::string prefix = "/Drive/Positions/Module" + std::to_string(i);
    ASSERT_TRUE(table.Contains(prefix + "/Distance")) << "Missing " << prefix + "/Distance";
    ASSERT_TRUE(table.Contains(prefix + "/Angle")) << "Missing " << prefix + "/Angle";

    EXPECT_DOUBLE_EQ(table.Get(prefix + "/Distance")->Get<double>(), static_cast<double>((i + 1) * 5));
  }
}

// Test that helper functions don't interfere with each other
TEST(StructLoggerTest, MultipleStructsInSameTable) {
  LogTable table;

  frc::Pose2d pose{1.0_m, 2.0_m, frc::Rotation2d(45_deg)};
  frc::Translation2d translation{3.0_m, 4.0_m};
  frc::SwerveModuleState state{5.0_mps, frc::Rotation2d(90_deg)};

  LogPose2d(table, "/Pose", pose);
  LogTranslation2d(table, "/Translation", translation);
  LogSwerveModuleState(table, "/State", state);

  // Verify all entries are present
  EXPECT_EQ(table.Size(), 7u);  // 3 + 2 + 2 = 7 fields total

  // Verify they don't interfere
  EXPECT_DOUBLE_EQ(table.Get("/Pose/X")->Get<double>(), 1.0);
  EXPECT_DOUBLE_EQ(table.Get("/Translation/X")->Get<double>(), 3.0);
  EXPECT_DOUBLE_EQ(table.Get("/State/Speed")->Get<double>(), 5.0);
}

// Test empty arrays
TEST(StructLoggerTest, EmptyArrays) {
  LogTable table;
  std::vector<frc::SwerveModuleState> emptyStates;
  std::vector<frc::SwerveModulePosition> emptyPositions;

  LogSwerveModuleStates(table, "/Empty/States", emptyStates);
  LogSwerveModulePositions(table, "/Empty/Positions", emptyPositions);

  // Should not create any entries for empty arrays
  EXPECT_EQ(table.Size(), 0u);
}

// Test key prefixing works correctly
TEST(StructLoggerTest, KeyPrefixing) {
  LogTable table;
  frc::Pose2d pose{1.0_m, 2.0_m, frc::Rotation2d(0_deg)};

  // Test with leading slash
  LogPose2d(table, "/Robot/Pose", pose);
  EXPECT_TRUE(table.Contains("/Robot/Pose/X"));
  EXPECT_TRUE(table.Contains("/Robot/Pose/Y"));

  // Test without leading slash (should still work)
  LogPose2d(table, "Autonomous/Pose", pose);
  EXPECT_TRUE(table.Contains("Autonomous/Pose/X"));
  EXPECT_TRUE(table.Contains("Autonomous/Pose/Y"));
}

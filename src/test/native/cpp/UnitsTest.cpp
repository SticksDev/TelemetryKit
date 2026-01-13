#include <gtest/gtest.h>

#include <string>

#include <units/angle.h>
#include <units/length.h>
#include <units/velocity.h>
#include <units/voltage.h>
#include <units/current.h>
#include <units/time.h>
#include <units/angular_velocity.h>

#include "telemetrykit/core/Units.h"
#include "telemetrykit/core/LogTable.h"

using namespace tkit;

// ============================================================================
// Tests for WPILib units integration
// ============================================================================

TEST(UnitsTest, RecordMeterUnit) {
  auto distance = units::meter_t{3.5};

  // Verify value extraction
  EXPECT_DOUBLE_EQ(distance.value(), 3.5);

  // Verify unit abbreviation (compare as strings)
  std::string abbrev = units::abbreviation(distance);
  EXPECT_EQ(abbrev, "m");
}

TEST(UnitsTest, RecordDegreeUnit) {
  auto angle = units::degree_t{90.0};

  EXPECT_DOUBLE_EQ(angle.value(), 90.0);
  std::string abbrev = units::abbreviation(angle);
  EXPECT_EQ(abbrev, "deg");
}

TEST(UnitsTest, RecordRadianUnit) {
  auto angle = units::radian_t{1.57};

  EXPECT_DOUBLE_EQ(angle.value(), 1.57);
  std::string abbrev = units::abbreviation(angle);
  EXPECT_EQ(abbrev, "rad");
}

TEST(UnitsTest, RecordVelocityUnit) {
  auto speed = units::meters_per_second_t{2.5};

  EXPECT_DOUBLE_EQ(speed.value(), 2.5);
  std::string abbrev = units::abbreviation(speed);
  // WPILib uses "mps" not "m/s"
  EXPECT_EQ(abbrev, "mps");
}

TEST(UnitsTest, RecordFeetPerSecondUnit) {
  auto speed = units::feet_per_second_t{10.0};

  EXPECT_DOUBLE_EQ(speed.value(), 10.0);
  std::string abbrev = units::abbreviation(speed);
  EXPECT_EQ(abbrev, "fps");
}

TEST(UnitsTest, RecordVoltageUnit) {
  auto voltage = units::volt_t{12.0};

  EXPECT_DOUBLE_EQ(voltage.value(), 12.0);
  std::string abbrev = units::abbreviation(voltage);
  EXPECT_EQ(abbrev, "V");
}

TEST(UnitsTest, RecordCurrentUnit) {
  auto current = units::ampere_t{42.0};

  EXPECT_DOUBLE_EQ(current.value(), 42.0);
  std::string abbrev = units::abbreviation(current);
  EXPECT_EQ(abbrev, "A");
}

TEST(UnitsTest, RecordTimeUnit) {
  auto time = units::second_t{1.5};

  EXPECT_DOUBLE_EQ(time.value(), 1.5);
  std::string abbrev = units::abbreviation(time);
  EXPECT_EQ(abbrev, "s");
}

TEST(UnitsTest, RecordMillisecondUnit) {
  auto time = units::millisecond_t{500.0};

  EXPECT_DOUBLE_EQ(time.value(), 500.0);
  std::string abbrev = units::abbreviation(time);
  EXPECT_EQ(abbrev, "ms");
}

TEST(UnitsTest, RecordAngularVelocityUnit) {
  auto angVel = units::radians_per_second_t{3.14};

  EXPECT_DOUBLE_EQ(angVel.value(), 3.14);
  std::string abbrev = units::abbreviation(angVel);
  EXPECT_EQ(abbrev, "rad_per_s");
}

TEST(UnitsTest, RecordRPMUnit) {
  auto rpm = units::revolutions_per_minute_t{6000.0};

  EXPECT_DOUBLE_EQ(rpm.value(), 6000.0);
  std::string abbrev = units::abbreviation(rpm);
  EXPECT_EQ(abbrev, "rpm");
}

TEST(UnitsTest, RecordInchUnit) {
  auto dist = units::inch_t{12.0};

  EXPECT_DOUBLE_EQ(dist.value(), 12.0);
  std::string abbrev = units::abbreviation(dist);
  EXPECT_EQ(abbrev, "in");
}

TEST(UnitsTest, RecordFootUnit) {
  auto dist = units::foot_t{3.0};

  EXPECT_DOUBLE_EQ(dist.value(), 3.0);
  std::string abbrev = units::abbreviation(dist);
  EXPECT_EQ(abbrev, "ft");
}

// ============================================================================
// Tests for Put helper function
// ============================================================================

TEST(UnitsTest, PutMeterToTable) {
  LogTable table;
  tkit::Put(table, "/Distance", units::meter_t{5.0});

  auto value = table.Get("/Distance");
  ASSERT_TRUE(value.has_value());
  EXPECT_DOUBLE_EQ(value->Get<double>(), 5.0);
  EXPECT_EQ(table.GetUnit("/Distance"), "m");
}

TEST(UnitsTest, PutDegreeToTable) {
  LogTable table;
  tkit::Put(table, "/Angle", units::degree_t{45.0});

  auto value = table.Get("/Angle");
  ASSERT_TRUE(value.has_value());
  EXPECT_DOUBLE_EQ(value->Get<double>(), 45.0);
  EXPECT_EQ(table.GetUnit("/Angle"), "deg");
}

TEST(UnitsTest, PutVelocityToTable) {
  LogTable table;
  tkit::Put(table, "/Speed", units::meters_per_second_t{3.5});

  auto value = table.Get("/Speed");
  ASSERT_TRUE(value.has_value());
  EXPECT_DOUBLE_EQ(value->Get<double>(), 3.5);
  // WPILib uses "mps" abbreviation
  EXPECT_EQ(table.GetUnit("/Speed"), "mps");
}

TEST(UnitsTest, PutVoltageToTable) {
  LogTable table;
  tkit::Put(table, "/Voltage", units::volt_t{12.6});

  auto value = table.Get("/Voltage");
  ASSERT_TRUE(value.has_value());
  EXPECT_DOUBLE_EQ(value->Get<double>(), 12.6);
  EXPECT_EQ(table.GetUnit("/Voltage"), "V");
}

TEST(UnitsTest, PutCurrentToTable) {
  LogTable table;
  tkit::Put(table, "/Current", units::ampere_t{30.0});

  auto value = table.Get("/Current");
  ASSERT_TRUE(value.has_value());
  EXPECT_DOUBLE_EQ(value->Get<double>(), 30.0);
  EXPECT_EQ(table.GetUnit("/Current"), "A");
}

TEST(UnitsTest, PutToSubtable) {
  LogTable rootTable;
  auto driveTable = rootTable.GetSubtable("/Drive");

  tkit::Put(driveTable, "LeftVelocity", units::meters_per_second_t{2.0});
  tkit::Put(driveTable, "RightVelocity", units::meters_per_second_t{2.1});

  EXPECT_TRUE(rootTable.Contains("/Drive/LeftVelocity"));
  EXPECT_TRUE(rootTable.Contains("/Drive/RightVelocity"));

  // WPILib uses "mps" abbreviation
  EXPECT_EQ(rootTable.GetUnit("/Drive/LeftVelocity"), "mps");
  EXPECT_EQ(rootTable.GetUnit("/Drive/RightVelocity"), "mps");
}

// ============================================================================
// Type trait tests
// ============================================================================

TEST(UnitsTest, IsUnitTypeDetection) {
  // Unit types should be detected
  EXPECT_TRUE(detail::is_unit_type_v<units::meter_t>);
  EXPECT_TRUE(detail::is_unit_type_v<units::degree_t>);
  EXPECT_TRUE(detail::is_unit_type_v<units::volt_t>);
  EXPECT_TRUE(detail::is_unit_type_v<units::meters_per_second_t>);

  // Non-unit types should not be detected
  EXPECT_FALSE(detail::is_unit_type_v<double>);
  EXPECT_FALSE(detail::is_unit_type_v<int>);
  EXPECT_FALSE(detail::is_unit_type_v<std::string>);
}

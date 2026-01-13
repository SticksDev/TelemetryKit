#include <gtest/gtest.h>
#include "telemetrykit/core/AutoLog.h"

using namespace tkit;

// Define test fields using X-macro pattern
#define DRIVE_INPUTS_FIELDS(X) \
  X(double, leftVelocity, 0.0, "m/s") \
  X(double, rightVelocity, 0.0, "m/s") \
  X(double, gyroAngle, 0.0, "radians") \
  X(bool, isMoving, false, "")

#define SIMPLE_FIELDS(X) \
  X(int64_t, count, 0, "") \
  X(double, value, 1.5, "volts") \
  X(bool, enabled, true, "")

#define MIXED_TYPES_FIELDS(X) \
  X(double, speed, 0.0, "m/s") \
  X(float, temperature, 25.0f, "celsius") \
  X(int64_t, tickCount, 0, "") \
  X(bool, limitSwitch, false, "") \
  X(std::string, status, "idle", "")

// Test TKIT_LOGGABLE_STRUCT macro
TKIT_LOGGABLE_STRUCT(DriveInputs, DRIVE_INPUTS_FIELDS);

// Test TKIT_LOGGABLE_CLASS macro
TKIT_LOGGABLE_CLASS(SimpleInputs, SIMPLE_FIELDS);

// Manual class using TKIT_DECLARE_LOGGABLE_FIELDS and TKIT_LOG_FIELDS
class ManualInputs : public LoggableInputs {
 public:
  TKIT_DECLARE_LOGGABLE_FIELDS(MIXED_TYPES_FIELDS)

  void ToLog(LogTable& table) const override {
    TKIT_LOG_FIELDS(MIXED_TYPES_FIELDS, table)
  }

  // Additional custom method
  void Reset() {
    speed = 0.0;
    temperature = 25.0f;
    tickCount = 0;
    limitSwitch = false;
    status = "idle";
  }
};

// ============================================================================
// Tests for TKIT_LOGGABLE_STRUCT
// ============================================================================

TEST(AutoLogTest, LosgableStructCreation) {
  DriveInputs inputs;

  // Check default values
  EXPECT_DOUBLE_EQ(inputs.leftVelocity, 0.0);
  EXPECT_DOUBLE_EQ(inputs.rightVelocity, 0.0);
  EXPECT_DOUBLE_EQ(inputs.gyroAngle, 0.0);
  EXPECT_FALSE(inputs.isMoving);
}

TEST(AutoLogTest, LoggableStructModification) {
  DriveInputs inputs;

  inputs.leftVelocity = 3.5;
  inputs.rightVelocity = 3.3;
  inputs.gyroAngle = 1.57;
  inputs.isMoving = true;

  EXPECT_DOUBLE_EQ(inputs.leftVelocity, 3.5);
  EXPECT_DOUBLE_EQ(inputs.rightVelocity, 3.3);
  EXPECT_DOUBLE_EQ(inputs.gyroAngle, 1.57);
  EXPECT_TRUE(inputs.isMoving);
}

TEST(AutoLogTest, LoggableStructToLog) {
  DriveInputs inputs;
  inputs.leftVelocity = 3.5;
  inputs.rightVelocity = 3.3;
  inputs.gyroAngle = 1.57;
  inputs.isMoving = true;

  LogTable table;
  inputs.ToLog(table);

  // Check values were logged
  auto leftVel = table.Get("leftVelocity");
  ASSERT_TRUE(leftVel.has_value());
  EXPECT_DOUBLE_EQ(leftVel->Get<double>(), 3.5);

  auto rightVel = table.Get("rightVelocity");
  ASSERT_TRUE(rightVel.has_value());
  EXPECT_DOUBLE_EQ(rightVel->Get<double>(), 3.3);

  auto angle = table.Get("gyroAngle");
  ASSERT_TRUE(angle.has_value());
  EXPECT_DOUBLE_EQ(angle->Get<double>(), 1.57);

  auto moving = table.Get("isMoving");
  ASSERT_TRUE(moving.has_value());
  EXPECT_TRUE(moving->Get<bool>());
}

TEST(AutoLogTest, LoggableStructUnitsLogged) {
  DriveInputs inputs;
  inputs.leftVelocity = 3.5;
  inputs.gyroAngle = 1.57;

  LogTable table;
  inputs.ToLog(table);

  // Check unit metadata was stored
  EXPECT_EQ(table.GetUnit("leftVelocity"), "m/s");
  EXPECT_EQ(table.GetUnit("rightVelocity"), "m/s");
  EXPECT_EQ(table.GetUnit("gyroAngle"), "radians");
  // Empty unit string should not be stored
  EXPECT_EQ(table.GetUnit("isMoving"), "");
}

TEST(AutoLogTest, LoggableStructInheritance) {
  DriveInputs inputs;

  // Should be a LoggableInputs
  LoggableInputs* base = &inputs;
  EXPECT_NE(base, nullptr);

  // Can call ToLog through base pointer
  LogTable table;
  base->ToLog(table);

  EXPECT_TRUE(table.Contains("leftVelocity"));
}

// ============================================================================
// Tests for TKIT_LOGGABLE_CLASS
// ============================================================================

TEST(AutoLogTest, LoggableClassCreation) {
  SimpleInputs inputs;

  // Check default values
  EXPECT_EQ(inputs.count, 0);
  EXPECT_DOUBLE_EQ(inputs.value, 1.5);
  EXPECT_TRUE(inputs.enabled);
}

TEST(AutoLogTest, LoggableClassToLog) {
  SimpleInputs inputs;
  inputs.count = 42;
  inputs.value = 12.6;
  inputs.enabled = false;

  LogTable table;
  inputs.ToLog(table);

  auto count = table.Get("count");
  ASSERT_TRUE(count.has_value());
  EXPECT_EQ(count->Get<int64_t>(), 42);

  auto value = table.Get("value");
  ASSERT_TRUE(value.has_value());
  EXPECT_DOUBLE_EQ(value->Get<double>(), 12.6);

  auto enabled = table.Get("enabled");
  ASSERT_TRUE(enabled.has_value());
  EXPECT_FALSE(enabled->Get<bool>());
}

TEST(AutoLogTest, LoggableClassUnitsLogged) {
  SimpleInputs inputs;

  LogTable table;
  inputs.ToLog(table);

  EXPECT_EQ(table.GetUnit("count"), "");
  EXPECT_EQ(table.GetUnit("value"), "volts");
  EXPECT_EQ(table.GetUnit("enabled"), "");
}

// ============================================================================
// Tests for manual class with TKIT_DECLARE_LOGGABLE_FIELDS
// ============================================================================

TEST(AutoLogTest, ManualClassCreation) {
  ManualInputs inputs;

  // Check default values
  EXPECT_DOUBLE_EQ(inputs.speed, 0.0);
  EXPECT_FLOAT_EQ(inputs.temperature, 25.0f);
  EXPECT_EQ(inputs.tickCount, 0);
  EXPECT_FALSE(inputs.limitSwitch);
  EXPECT_EQ(inputs.status, "idle");
}

TEST(AutoLogTest, ManualClassToLog) {
  ManualInputs inputs;
  inputs.speed = 5.5;
  inputs.temperature = 30.0f;
  inputs.tickCount = 1000;
  inputs.limitSwitch = true;
  inputs.status = "running";

  LogTable table;
  inputs.ToLog(table);

  auto speed = table.Get("speed");
  ASSERT_TRUE(speed.has_value());
  EXPECT_DOUBLE_EQ(speed->Get<double>(), 5.5);

  auto temp = table.Get("temperature");
  ASSERT_TRUE(temp.has_value());
  EXPECT_FLOAT_EQ(temp->Get<float>(), 30.0f);

  auto ticks = table.Get("tickCount");
  ASSERT_TRUE(ticks.has_value());
  EXPECT_EQ(ticks->Get<int64_t>(), 1000);

  auto limit = table.Get("limitSwitch");
  ASSERT_TRUE(limit.has_value());
  EXPECT_TRUE(limit->Get<bool>());

  auto status = table.Get("status");
  ASSERT_TRUE(status.has_value());
  EXPECT_EQ(status->Get<std::string>(), "running");
}

TEST(AutoLogTest, ManualClassUnitsLogged) {
  ManualInputs inputs;

  LogTable table;
  inputs.ToLog(table);

  EXPECT_EQ(table.GetUnit("speed"), "m/s");
  EXPECT_EQ(table.GetUnit("temperature"), "celsius");
  EXPECT_EQ(table.GetUnit("tickCount"), "");
  EXPECT_EQ(table.GetUnit("limitSwitch"), "");
  EXPECT_EQ(table.GetUnit("status"), "");
}

TEST(AutoLogTest, ManualClassCustomMethods) {
  ManualInputs inputs;
  inputs.speed = 10.0;
  inputs.tickCount = 500;

  inputs.Reset();

  EXPECT_DOUBLE_EQ(inputs.speed, 0.0);
  EXPECT_EQ(inputs.tickCount, 0);
}

// ============================================================================
// Tests for logging to subtables
// ============================================================================

TEST(AutoLogTest, LogToSubtable) {
  DriveInputs inputs;
  inputs.leftVelocity = 3.5;
  inputs.rightVelocity = 3.3;

  LogTable rootTable;
  auto driveTable = rootTable.GetSubtable("/Drive");

  inputs.ToLog(driveTable);

  // Values should be stored with prefix
  EXPECT_TRUE(rootTable.Contains("/Drive/leftVelocity"));
  EXPECT_TRUE(rootTable.Contains("/Drive/rightVelocity"));

  auto leftVel = rootTable.Get("/Drive/leftVelocity");
  ASSERT_TRUE(leftVel.has_value());
  EXPECT_DOUBLE_EQ(leftVel->Get<double>(), 3.5);

  // Units should also have the prefix
  EXPECT_EQ(rootTable.GetUnit("/Drive/leftVelocity"), "m/s");
}

TEST(AutoLogTest, LogToNestedSubtable) {
  SimpleInputs inputs;
  inputs.count = 100;
  inputs.value = 5.0;

  LogTable rootTable;
  auto subsystemTable = rootTable.GetSubtable("/Subsystems");
  auto armTable = subsystemTable.GetSubtable("/Arm");

  inputs.ToLog(armTable);

  // Values should be stored with nested prefix
  EXPECT_TRUE(rootTable.Contains("/Subsystems/Arm/count"));
  EXPECT_TRUE(rootTable.Contains("/Subsystems/Arm/value"));

  auto count = rootTable.Get("/Subsystems/Arm/count");
  ASSERT_TRUE(count.has_value());
  EXPECT_EQ(count->Get<int64_t>(), 100);
}

// ============================================================================
// Tests for multiple log calls
// ============================================================================

TEST(AutoLogTest, MultipleLogCalls) {
  DriveInputs inputs;
  LogTable table;

  // First log
  inputs.leftVelocity = 1.0;
  inputs.ToLog(table);

  auto val1 = table.Get("leftVelocity");
  ASSERT_TRUE(val1.has_value());
  EXPECT_DOUBLE_EQ(val1->Get<double>(), 1.0);

  // Second log with updated value
  inputs.leftVelocity = 2.0;
  inputs.ToLog(table);

  auto val2 = table.Get("leftVelocity");
  ASSERT_TRUE(val2.has_value());
  EXPECT_DOUBLE_EQ(val2->Get<double>(), 2.0);

  // Should still be only 4 entries (not duplicated)
  EXPECT_EQ(table.Size(), 4u);
}

// ============================================================================
// Tests for GetAllUnits
// ============================================================================

TEST(AutoLogTest, GetAllUnitsFromLoggedStruct) {
  DriveInputs inputs;
  inputs.leftVelocity = 3.5;

  LogTable table;
  inputs.ToLog(table);

  auto units = table.GetAllUnits();

  // Should have units for fields with non-empty unit strings
  EXPECT_EQ(units.size(), 3u);  // leftVelocity, rightVelocity, gyroAngle
  EXPECT_EQ(units["/leftVelocity"], "m/s");
  EXPECT_EQ(units["/rightVelocity"], "m/s");
  EXPECT_EQ(units["/gyroAngle"], "radians");
}

// ============================================================================
// Edge case tests
// ============================================================================

TEST(AutoLogTest, ZeroDefaultValues) {
  // Define fields with zero/empty defaults
  #define ZERO_FIELDS(X) \
    X(double, zeroDouble, 0.0, "") \
    X(int64_t, zeroInt, 0, "") \
    X(bool, falseBool, false, "")

  TKIT_LOGGABLE_STRUCT(ZeroInputs, ZERO_FIELDS);

  ZeroInputs inputs;

  EXPECT_DOUBLE_EQ(inputs.zeroDouble, 0.0);
  EXPECT_EQ(inputs.zeroInt, 0);
  EXPECT_FALSE(inputs.falseBool);

  LogTable table;
  inputs.ToLog(table);

  // Should still log zero values
  auto zd = table.Get("zeroDouble");
  ASSERT_TRUE(zd.has_value());
  EXPECT_DOUBLE_EQ(zd->Get<double>(), 0.0);
}

TEST(AutoLogTest, SingleField) {
  // Define a struct with just one field
  #define SINGLE_FIELD(X) \
    X(double, onlyField, 42.0, "units")

  TKIT_LOGGABLE_STRUCT(SingleFieldInputs, SINGLE_FIELD);

  SingleFieldInputs inputs;

  EXPECT_DOUBLE_EQ(inputs.onlyField, 42.0);

  LogTable table;
  inputs.ToLog(table);

  EXPECT_EQ(table.Size(), 1u);
  EXPECT_EQ(table.GetUnit("onlyField"), "units");
}

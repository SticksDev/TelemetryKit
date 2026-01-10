#include <gtest/gtest.h>
#include <thread>
#include "telemetrykit/core/LogTable.h"

using namespace tkit;

// Basic operations
TEST(LogTableTest, PutAndGet) {
  LogTable table;
  table.Put("/Speed", 3.5);

  auto value = table.Get("/Speed");
  ASSERT_TRUE(value.has_value());
  EXPECT_DOUBLE_EQ(value->Get<double>(), 3.5);
}

TEST(LogTableTest, PutWithoutLeadingSlash) {
  LogTable table;
  table.Put("Speed", 3.5);

  // Should normalize to "/Speed"
  auto value = table.Get("/Speed");
  ASSERT_TRUE(value.has_value());
  EXPECT_DOUBLE_EQ(value->Get<double>(), 3.5);
}

TEST(LogTableTest, GetNonExistent) {
  LogTable table;
  auto value = table.Get("/DoesNotExist");
  EXPECT_FALSE(value.has_value());
}

TEST(LogTableTest, Contains) {
  LogTable table;
  table.Put("/Speed", 3.5);

  EXPECT_TRUE(table.Contains("/Speed"));
  EXPECT_FALSE(table.Contains("/Velocity"));
}

TEST(LogTableTest, Remove) {
  LogTable table;
  table.Put("/Speed", 3.5);
  EXPECT_TRUE(table.Contains("/Speed"));

  table.Remove("/Speed");
  EXPECT_FALSE(table.Contains("/Speed"));
}

TEST(LogTableTest, Clear) {
  LogTable table;
  table.Put("/Speed", 3.5);
  table.Put("/Velocity", 2.5);
  EXPECT_EQ(table.Size(), 2u);

  table.Clear();
  EXPECT_EQ(table.Size(), 0u);
  EXPECT_TRUE(table.IsEmpty());
}

TEST(LogTableTest, Size) {
  LogTable table;
  EXPECT_EQ(table.Size(), 0u);
  EXPECT_TRUE(table.IsEmpty());

  table.Put("/A", 1);
  EXPECT_EQ(table.Size(), 1u);
  EXPECT_FALSE(table.IsEmpty());

  table.Put("/B", 2);
  table.Put("/C", 3);
  EXPECT_EQ(table.Size(), 3u);
}

// Hierarchical keys
TEST(LogTableTest, HierarchicalKeys) {
  LogTable table;
  table.Put("/Drivetrain/LeftMotor/Velocity", 3.5);
  table.Put("/Drivetrain/RightMotor/Velocity", 3.3);
  table.Put("/Arm/Position", 1.57);

  EXPECT_EQ(table.Size(), 3u);
  EXPECT_TRUE(table.Contains("/Drivetrain/LeftMotor/Velocity"));
  EXPECT_TRUE(table.Contains("/Drivetrain/RightMotor/Velocity"));
  EXPECT_TRUE(table.Contains("/Arm/Position"));
}

// Subtables
TEST(LogTableTest, Subtable) {
  LogTable rootTable;

  // Create a subtable
  auto gyroTable = rootTable.GetSubtable("/Gyro");
  EXPECT_EQ(gyroTable.GetPrefix(), "/Gyro");

  // Put values through subtable
  gyroTable.Put("Yaw", 45.0);
  gyroTable.Put("Pitch", 10.0);

  // Verify they appear in root table with full key
  EXPECT_TRUE(rootTable.Contains("/Gyro/Yaw"));
  EXPECT_TRUE(rootTable.Contains("/Gyro/Pitch"));

  auto yaw = rootTable.Get("/Gyro/Yaw");
  ASSERT_TRUE(yaw.has_value());
  EXPECT_DOUBLE_EQ(yaw->Get<double>(), 45.0);
}

TEST(LogTableTest, NestedSubtables) {
  LogTable rootTable;

  auto driveTable = rootTable.GetSubtable("/Drivetrain");
  auto leftTable = driveTable.GetSubtable("Left");

  leftTable.Put("Velocity", 3.5);

  // Should be stored as "/Drivetrain/Left/Velocity"
  EXPECT_TRUE(rootTable.Contains("/Drivetrain/Left/Velocity"));
}

TEST(LogTableTest, SubtableSize) {
  LogTable rootTable;
  rootTable.Put("/Drivetrain/Left/Velocity", 3.5);
  rootTable.Put("/Drivetrain/Right/Velocity", 3.3);
  rootTable.Put("/Arm/Position", 1.57);

  auto driveTable = rootTable.GetSubtable("/Drivetrain");

  // Subtable should only count entries under its prefix
  EXPECT_EQ(driveTable.Size(), 2u);
  EXPECT_EQ(rootTable.Size(), 3u);
}

TEST(LogTableTest, SubtableClear) {
  LogTable rootTable;
  rootTable.Put("/Drivetrain/Left/Velocity", 3.5);
  rootTable.Put("/Drivetrain/Right/Velocity", 3.3);
  rootTable.Put("/Arm/Position", 1.57);

  auto driveTable = rootTable.GetSubtable("/Drivetrain");
  driveTable.Clear();

  // Drivetrain entries should be cleared
  EXPECT_EQ(driveTable.Size(), 0u);
  EXPECT_FALSE(rootTable.Contains("/Drivetrain/Left/Velocity"));
  EXPECT_FALSE(rootTable.Contains("/Drivetrain/Right/Velocity"));

  // Arm entry should remain
  EXPECT_TRUE(rootTable.Contains("/Arm/Position"));
  EXPECT_EQ(rootTable.Size(), 1u);
}

// GetAllEntries
TEST(LogTableTest, GetAllEntries) {
  LogTable table;
  table.Put("/A", 1);
  table.Put("/B", 2);
  table.Put("/C", 3);

  auto entries = table.GetAllEntries();
  EXPECT_EQ(entries.size(), 3u);
  EXPECT_TRUE(entries.contains("/A"));
  EXPECT_TRUE(entries.contains("/B"));
  EXPECT_TRUE(entries.contains("/C"));
}

TEST(LogTableTest, GetAllEntriesSubtable) {
  LogTable rootTable;
  rootTable.Put("/Drivetrain/Left", 1.0);
  rootTable.Put("/Drivetrain/Right", 2.0);
  rootTable.Put("/Arm/Position", 3.0);

  auto driveTable = rootTable.GetSubtable("/Drivetrain");
  auto entries = driveTable.GetAllEntries();

  // Should only get Drivetrain entries
  EXPECT_EQ(entries.size(), 2u);
  EXPECT_TRUE(entries.contains("/Drivetrain/Left"));
  EXPECT_TRUE(entries.contains("/Drivetrain/Right"));
  EXPECT_FALSE(entries.contains("/Arm/Position"));
}

// GetKeys
TEST(LogTableTest, GetKeys) {
  LogTable table;
  table.Put("/A", 1);
  table.Put("/B", 2);
  table.Put("/C", 3);

  auto keys = table.GetKeys();
  EXPECT_EQ(keys.size(), 3u);

  // Check all keys present (order doesn't matter)
  EXPECT_NE(std::find(keys.begin(), keys.end(), "/A"), keys.end());
  EXPECT_NE(std::find(keys.begin(), keys.end(), "/B"), keys.end());
  EXPECT_NE(std::find(keys.begin(), keys.end(), "/C"), keys.end());
}

// Change detection
TEST(LogTableTest, HasChanged) {
  LogTable table;

  // Key doesn't exist yet
  EXPECT_TRUE(table.HasChanged("/Speed", LogValue(3.5)));

  // Add the key
  table.Put("/Speed", 3.5);

  // Same value, no change
  EXPECT_FALSE(table.HasChanged("/Speed", LogValue(3.5)));

  // Different value, has changed
  EXPECT_TRUE(table.HasChanged("/Speed", LogValue(4.0)));
}

// Template Put method
TEST(LogTableTest, TemplatePut) {
  LogTable table;

  // Test various types
  table.Put("/Bool", true);
  table.Put("/Int", 42);
  table.Put("/Float", 3.14f);
  table.Put("/Double", 2.718);
  table.Put("/String", std::string("Hello"));

  EXPECT_TRUE(table.Get("/Bool")->Is<bool>());
  EXPECT_TRUE(table.Get("/Int")->Is<int64_t>());
  EXPECT_TRUE(table.Get("/Float")->Is<float>());
  EXPECT_TRUE(table.Get("/Double")->Is<double>());
  EXPECT_TRUE(table.Get("/String")->Is<std::string>());
}

// Thread safety (basic test)
TEST(LogTableTest, ConcurrentReads) {
  LogTable table;
  table.Put("/Value", 42);

  // Multiple concurrent readers
  std::vector<std::thread> threads;
  std::atomic<int> successCount{0};

  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&table, &successCount]() {
      for (int j = 0; j < 100; ++j) {
        auto value = table.Get("/Value");
        if (value.has_value() && value->Get<int64_t>() == 42) {
          successCount++;
        }
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  EXPECT_EQ(successCount, 1000);  // 10 threads * 100 reads
}

TEST(LogTableTest, ConcurrentWrites) {
  LogTable table;

  // Multiple concurrent writers
  std::vector<std::thread> threads;

  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&table, i]() {
      for (int j = 0; j < 100; ++j) {
        table.Put("/Key" + std::to_string(i), j);
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  // Should have 10 keys (one per thread)
  EXPECT_EQ(table.Size(), 10u);
}

// Overwrite values
TEST(LogTableTest, OverwriteValue) {
  LogTable table;
  table.Put("/Speed", 3.5);

  auto value1 = table.Get("/Speed");
  ASSERT_TRUE(value1.has_value());
  EXPECT_DOUBLE_EQ(value1->Get<double>(), 3.5);

  // Overwrite with new value
  table.Put("/Speed", 5.0);

  auto value2 = table.Get("/Speed");
  ASSERT_TRUE(value2.has_value());
  EXPECT_DOUBLE_EQ(value2->Get<double>(), 5.0);

  // Still only one entry
  EXPECT_EQ(table.Size(), 1u);
}

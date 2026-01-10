#include <gtest/gtest.h>
#include "telemetrykit/core/Logger.h"
#include "telemetrykit/receiver/LogDataReceiver.h"
#include "telemetrykit/core/LoggableInputs.h"

using namespace tkit;

// Mock receiver for testing
class MockReceiver : public LogDataReceiver {
 public:
  int startCount{0};
  int updateCount{0};
  int endCount{0};
  LogTable lastTable;
  int64_t lastTimestamp{0};

  void OnStart() override {
    startCount++;
  }

  void OnUpdate(const LogTable& table, int64_t timestamp) override {
    updateCount++;
    lastTable = table;
    lastTimestamp = timestamp;
  }

  void OnEnd() override {
    endCount++;
  }
};

// Test fixture that cleans up after each test
class LoggerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Get logger instance
    auto& logger = Logger::GetInstance();

    // Clean up from any previous tests
    logger.RemoveAllReceivers();

    // Ensure stopped
    if (logger.IsLogging()) {
      logger.End();
    }
  }

  void TearDown() override {
    auto& logger = Logger::GetInstance();
    logger.RemoveAllReceivers();
    if (logger.IsLogging()) {
      logger.End();
    }
  }
};

// Singleton
TEST_F(LoggerTest, Singleton) {
  Logger& logger1 = Logger::GetInstance();
  Logger& logger2 = Logger::GetInstance();

  EXPECT_EQ(&logger1, &logger2);
}

// Lifecycle
TEST_F(LoggerTest, StartAndEnd) {
  auto& logger = Logger::GetInstance();

  EXPECT_FALSE(logger.IsLogging());

  logger.Start();
  EXPECT_TRUE(logger.IsLogging());

  logger.End();
  EXPECT_FALSE(logger.IsLogging());
}

TEST_F(LoggerTest, StartIdempotent) {
  auto& logger = Logger::GetInstance();

  logger.Start();
  EXPECT_TRUE(logger.IsLogging());

  logger.Start();  // Second call should be safe
  EXPECT_TRUE(logger.IsLogging());

  logger.End();
}

// Receiver management
TEST_F(LoggerTest, AddReceiver) {
  auto& logger = Logger::GetInstance();
  auto mockReceiver = std::make_unique<MockReceiver>();
  auto* mockPtr = mockReceiver.get();

  logger.AddReceiver(std::move(mockReceiver));
  logger.Start();

  EXPECT_EQ(mockPtr->startCount, 1);

  logger.End();
}

TEST_F(LoggerTest, RemoveAllReceivers) {
  auto& logger = Logger::GetInstance();

  logger.AddReceiver(std::make_unique<MockReceiver>());
  logger.AddReceiver(std::make_unique<MockReceiver>());

  logger.RemoveAllReceivers();

  logger.Start();
  logger.End();

  // No receivers to call, so no crashes
}

// Recording
TEST_F(LoggerTest, RecordOutput) {
  auto& logger = Logger::GetInstance();

  logger.RecordOutput("/Speed", 3.5);
  logger.RecordOutput("/Enabled", true);
  logger.RecordOutput("/Name", std::string("Robot"));

  auto& table = logger.GetRootTable();
  EXPECT_TRUE(table.Contains("/Speed"));
  EXPECT_TRUE(table.Contains("/Enabled"));
  EXPECT_TRUE(table.Contains("/Name"));

  auto speed = table.Get("/Speed");
  ASSERT_TRUE(speed.has_value());
  EXPECT_DOUBLE_EQ(speed->Get<double>(), 3.5);
}

TEST_F(LoggerTest, RecordOutputConvenience) {
  auto& logger = Logger::GetInstance();

  // Test convenience function
  tkit::RecordOutput("/Test", 42);

  auto& table = logger.GetRootTable();
  auto value = table.Get("/Test");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(value->Get<int64_t>(), 42);
}

// Periodic updates
TEST_F(LoggerTest, PeriodicCallsReceivers) {
  auto& logger = Logger::GetInstance();

  auto mockReceiver = std::make_unique<MockReceiver>();
  auto* mockPtr = mockReceiver.get();

  logger.AddReceiver(std::move(mockReceiver));
  logger.Start();

  logger.RecordOutput("/Speed", 3.5);
  logger.Periodic();

  EXPECT_EQ(mockPtr->updateCount, 1);
  EXPECT_GT(mockPtr->lastTimestamp, 0);

  // Verify the receiver got the logged data
  EXPECT_TRUE(mockPtr->lastTable.Contains("/Speed"));

  logger.End();
}

TEST_F(LoggerTest, MultiplePeriodicCalls) {
  auto& logger = Logger::GetInstance();

  auto mockReceiver = std::make_unique<MockReceiver>();
  auto* mockPtr = mockReceiver.get();

  logger.AddReceiver(std::move(mockReceiver));
  logger.Start();

  logger.Periodic();
  logger.Periodic();
  logger.Periodic();

  EXPECT_EQ(mockPtr->updateCount, 3);

  logger.End();
}

// Subtables
TEST_F(LoggerTest, GetTable) {
  auto& logger = Logger::GetInstance();

  auto gyroTable = logger.GetTable("/Gyro");
  gyroTable.Put("Yaw", 45.0);
  gyroTable.Put("Pitch", 10.0);

  auto& rootTable = logger.GetRootTable();
  EXPECT_TRUE(rootTable.Contains("/Gyro/Yaw"));
  EXPECT_TRUE(rootTable.Contains("/Gyro/Pitch"));
}

// LoggableInputs
TEST_F(LoggerTest, ProcessInputs) {
  struct TestInputs : LoggableInputs {
    double yaw{0.0};
    bool connected{false};

    void ToLog(LogTable& table) const override {
      table.Put("Yaw", yaw);
      table.Put("Connected", connected);
    }
  };

  auto& logger = Logger::GetInstance();

  TestInputs inputs;
  inputs.yaw = 45.0;
  inputs.connected = true;

  logger.ProcessInputs("/Gyro", inputs);

  auto& rootTable = logger.GetRootTable();
  EXPECT_TRUE(rootTable.Contains("/Gyro/Yaw"));
  EXPECT_TRUE(rootTable.Contains("/Gyro/Connected"));

  auto yaw = rootTable.Get("/Gyro/Yaw");
  ASSERT_TRUE(yaw.has_value());
  EXPECT_DOUBLE_EQ(yaw->Get<double>(), 45.0);
}

// Receiver lifecycle
TEST_F(LoggerTest, ReceiverLifecycle) {
  auto& logger = Logger::GetInstance();

  auto mockReceiver = std::make_unique<MockReceiver>();
  auto* mockPtr = mockReceiver.get();

  logger.AddReceiver(std::move(mockReceiver));

  // Start logging
  logger.Start();
  EXPECT_EQ(mockPtr->startCount, 1);

  // Periodic updates
  logger.Periodic();
  logger.Periodic();
  EXPECT_EQ(mockPtr->updateCount, 2);

  // End logging
  logger.End();
  EXPECT_EQ(mockPtr->endCount, 1);
}

// Multiple receivers
TEST_F(LoggerTest, MultipleReceivers) {
  auto& logger = Logger::GetInstance();

  auto mock1 = std::make_unique<MockReceiver>();
  auto mock2 = std::make_unique<MockReceiver>();
  auto* mockPtr1 = mock1.get();
  auto* mockPtr2 = mock2.get();

  logger.AddReceiver(std::move(mock1));
  logger.AddReceiver(std::move(mock2));

  logger.Start();

  EXPECT_EQ(mockPtr1->startCount, 1);
  EXPECT_EQ(mockPtr2->startCount, 1);

  logger.Periodic();

  EXPECT_EQ(mockPtr1->updateCount, 1);
  EXPECT_EQ(mockPtr2->updateCount, 1);

  logger.End();

  EXPECT_EQ(mockPtr1->endCount, 1);
  EXPECT_EQ(mockPtr2->endCount, 1);
}

// Timestamp
TEST_F(LoggerTest, GetTimestampUs) {
  int64_t timestamp = Logger::GetTimestampUs();
  EXPECT_GT(timestamp, 0);
}

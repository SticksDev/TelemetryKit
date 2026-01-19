#include <gtest/gtest.h>

#include "telemetrykit/core/AlertManager.h"
#include "telemetrykit/core/Logger.h"
#include "telemetrykit/receiver/LogDataReceiver.h"

using namespace tkit;

// Mock receiver for testing (minimal implementation)
class MockAlertReceiver : public LogDataReceiver {
 public:
  void OnStart() override {}
  void OnUpdate(const LogTable& table, int64_t timestamp) override {}
  void OnEnd() override {}
};

// Test fixture
class AlertManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto& alerts = AlertManager::GetInstance();
    alerts.Clear();

    auto& logger = Logger::GetInstance();
    logger.RemoveAllReceivers();
    if (logger.IsLogging()) {
      logger.End();
    }
    // Clear any logged values
    logger.GetRootTable().Clear();
  }

  void TearDown() override {
    auto& alerts = AlertManager::GetInstance();
    alerts.Clear();

    auto& logger = Logger::GetInstance();
    logger.RemoveAllReceivers();
    if (logger.IsLogging()) {
      logger.End();
    }
  }
};

// ============================================================================
// Singleton Tests
// ============================================================================

TEST_F(AlertManagerTest, Singleton) {
  AlertManager& alerts1 = AlertManager::GetInstance();
  AlertManager& alerts2 = AlertManager::GetInstance();

  EXPECT_EQ(&alerts1, &alerts2);
}

// ============================================================================
// Manual Alert Tests
// ============================================================================

TEST_F(AlertManagerTest, ManualInfoAlert) {
  auto& alerts = AlertManager::GetInstance();

  // Should not throw
  alerts.Info("TestKey", "Test info message");
  alerts.Info("TestKey", "Test info message", false);  // Clear
}

TEST_F(AlertManagerTest, ManualWarningAlert) {
  auto& alerts = AlertManager::GetInstance();

  alerts.Warning("TestWarning", "Test warning message");
  alerts.Warning("TestWarning", "Test warning message", false);
}

TEST_F(AlertManagerTest, ManualErrorAlert) {
  auto& alerts = AlertManager::GetInstance();

  alerts.Error("TestError", "Test error message");
  alerts.Error("TestError", "Test error message", false);
}

TEST_F(AlertManagerTest, MultipleManualAlerts) {
  auto& alerts = AlertManager::GetInstance();

  alerts.Info("Info1", "Info 1");
  alerts.Warning("Warn1", "Warning 1");
  alerts.Error("Error1", "Error 1");

  // All should coexist without issues
  alerts.Info("Info1", "Info 1", false);
  alerts.Warning("Warn1", "Warning 1", false);
  alerts.Error("Error1", "Error 1", false);
}

// ============================================================================
// Auto-Monitored Alert Tests
// ============================================================================

TEST_F(AlertManagerTest, AddThresholdAlertAbove) {
  auto& alerts = AlertManager::GetInstance();

  // Should not throw
  alerts.AddThresholdAlert("/Test/Value", "Value too high", 10.0,
                  AlertCondition::kAbove, frc::Alert::AlertType::kWarning);
}

TEST_F(AlertManagerTest, AddThresholdAlertBelow) {
  auto& alerts = AlertManager::GetInstance();

  alerts.AddThresholdAlert("/Test/Value", "Value too low", 5.0,
                  AlertCondition::kBelow, frc::Alert::AlertType::kError);
}

TEST_F(AlertManagerTest, AddRangeAlert) {
  auto& alerts = AlertManager::GetInstance();

  alerts.AddRangeAlert("/Test/Value", "Value out of range", 0.0, 100.0,
                       frc::Alert::AlertType::kWarning);
}

TEST_F(AlertManagerTest, AddOnChangeAlertDouble) {
  auto& alerts = AlertManager::GetInstance();

  alerts.AddOnChangeAlert("/Test/State", "State changed", 0.0,
                          frc::Alert::AlertType::kWarning);
}

TEST_F(AlertManagerTest, AddOnChangeAlertBool) {
  auto& alerts = AlertManager::GetInstance();

  alerts.AddOnChangeAlert("/Test/Flag", "Flag changed", false,
                          frc::Alert::AlertType::kError);
}

// ============================================================================
// CheckAll Tests (with LogTable integration)
// ============================================================================

TEST_F(AlertManagerTest, CheckAllWithAboveThreshold) {
  auto& alerts = AlertManager::GetInstance();
  auto& logger = Logger::GetInstance();

  // Add receiver and start logger
  logger.AddReceiver(std::make_unique<MockAlertReceiver>());
  logger.Start();

  // Add alert for value > 10
  alerts.AddThresholdAlert("/Test/Value", "Value too high", 10.0,
                  AlertCondition::kAbove, frc::Alert::AlertType::kWarning);

  // Log a value below threshold
  logger.RecordOutput("/Test/Value", 5.0);
  alerts.CheckAll();  // Should not trigger

  // Log a value above threshold
  logger.RecordOutput("/Test/Value", 15.0);
  alerts.CheckAll();  // Should trigger

  logger.End();
}

TEST_F(AlertManagerTest, CheckAllWithBelowThreshold) {
  auto& alerts = AlertManager::GetInstance();
  auto& logger = Logger::GetInstance();

  logger.AddReceiver(std::make_unique<MockAlertReceiver>());
  logger.Start();

  // Add alert for value < 10
  alerts.AddThresholdAlert("/Test/Value", "Value too low", 10.0,
                  AlertCondition::kBelow, frc::Alert::AlertType::kWarning);

  // Log a value above threshold
  logger.RecordOutput("/Test/Value", 15.0);
  alerts.CheckAll();  // Should not trigger

  // Log a value below threshold
  logger.RecordOutput("/Test/Value", 5.0);
  alerts.CheckAll();  // Should trigger

  logger.End();
}

TEST_F(AlertManagerTest, CheckAllWithRangeAlert) {
  auto& alerts = AlertManager::GetInstance();
  auto& logger = Logger::GetInstance();

  logger.AddReceiver(std::make_unique<MockAlertReceiver>());
  logger.Start();

  // Add alert for value outside [0, 100]
  alerts.AddRangeAlert("/Test/Value", "Value out of range", 0.0, 100.0,
                       frc::Alert::AlertType::kWarning);

  // Log a value in range
  logger.RecordOutput("/Test/Value", 50.0);
  alerts.CheckAll();  // Should not trigger

  // Log a value below range
  logger.RecordOutput("/Test/Value", -10.0);
  alerts.CheckAll();  // Should trigger

  // Log a value above range
  logger.RecordOutput("/Test/Value", 150.0);
  alerts.CheckAll();  // Should trigger

  logger.End();
}

TEST_F(AlertManagerTest, CheckAllWithOnChangeDouble) {
  auto& alerts = AlertManager::GetInstance();
  auto& logger = Logger::GetInstance();

  logger.AddReceiver(std::make_unique<MockAlertReceiver>());
  logger.Start();

  // Add alert expecting value == 0
  alerts.AddOnChangeAlert("/Test/State", "State changed", 0.0,
                          frc::Alert::AlertType::kWarning);

  // Log expected value
  logger.RecordOutput("/Test/State", 0.0);
  alerts.CheckAll();  // Should not trigger

  // Log unexpected value
  logger.RecordOutput("/Test/State", 1.0);
  alerts.CheckAll();  // Should trigger

  logger.End();
}

TEST_F(AlertManagerTest, CheckAllWithOnChangeBool) {
  auto& alerts = AlertManager::GetInstance();
  auto& logger = Logger::GetInstance();

  logger.AddReceiver(std::make_unique<MockAlertReceiver>());
  logger.Start();

  // Add alert expecting false
  alerts.AddOnChangeAlert("/Test/Flag", "Flag changed", false,
                          frc::Alert::AlertType::kWarning);

  // Log expected value
  logger.RecordOutput("/Test/Flag", false);
  alerts.CheckAll();  // Should not trigger

  // Log unexpected value
  logger.RecordOutput("/Test/Flag", true);
  alerts.CheckAll();  // Should trigger

  logger.End();
}

TEST_F(AlertManagerTest, CheckAllWithMissingKey) {
  auto& alerts = AlertManager::GetInstance();
  auto& logger = Logger::GetInstance();

  logger.AddReceiver(std::make_unique<MockAlertReceiver>());
  logger.Start();

  // Add alert for key that doesn't exist
  alerts.AddThresholdAlert("/NonExistent/Key", "Missing", 10.0,
                  AlertCondition::kAbove, frc::Alert::AlertType::kWarning);

  // Should not throw when checking
  alerts.CheckAll();

  logger.End();
}

TEST_F(AlertManagerTest, CheckAllWithDifferentNumericTypes) {
  auto& alerts = AlertManager::GetInstance();
  auto& logger = Logger::GetInstance();

  logger.AddReceiver(std::make_unique<MockAlertReceiver>());
  logger.Start();

  alerts.AddThresholdAlert("/Test/Int", "Int too high", 10.0,
                  AlertCondition::kAbove, frc::Alert::AlertType::kWarning);
  alerts.AddThresholdAlert("/Test/Float", "Float too high", 10.0,
                  AlertCondition::kAbove, frc::Alert::AlertType::kWarning);

  // Log int64
  logger.RecordOutput("/Test/Int", static_cast<int64_t>(15));
  alerts.CheckAll();

  // Log float
  logger.RecordOutput("/Test/Float", 15.0f);
  alerts.CheckAll();

  logger.End();
}

// ============================================================================
// Clear Tests
// ============================================================================

TEST_F(AlertManagerTest, ClearManualAlert) {
  auto& alerts = AlertManager::GetInstance();

  alerts.Info("TestAlert", "Test message");

  bool removed = alerts.ClearManualAlert("TestAlert");
  EXPECT_TRUE(removed);

  // Second removal should return false
  removed = alerts.ClearManualAlert("TestAlert");
  EXPECT_FALSE(removed);
}

TEST_F(AlertManagerTest, ClearAutoAlert) {
  auto& alerts = AlertManager::GetInstance();

  alerts.AddThresholdAlert("/Test/Value", "Alert 1", 10.0,
                  AlertCondition::kAbove, frc::Alert::AlertType::kWarning);

  bool removed = alerts.ClearAutoAlert("/Test/Value");
  EXPECT_TRUE(removed);

  // Second removal should return false
  removed = alerts.ClearAutoAlert("/Test/Value");
  EXPECT_FALSE(removed);
}

TEST_F(AlertManagerTest, ClearAutoAlertMultipleSameKey) {
  auto& alerts = AlertManager::GetInstance();

  // Add multiple alerts for the same key
  alerts.AddThresholdAlert("/Test/Value", "Alert 1", 10.0,
                  AlertCondition::kAbove, frc::Alert::AlertType::kWarning);
  alerts.AddThresholdAlert("/Test/Value", "Alert 2", 5.0,
                  AlertCondition::kBelow, frc::Alert::AlertType::kError);

  // Should remove both
  bool removed = alerts.ClearAutoAlert("/Test/Value");
  EXPECT_TRUE(removed);

  // Second removal should return false (both already removed)
  removed = alerts.ClearAutoAlert("/Test/Value");
  EXPECT_FALSE(removed);
}

TEST_F(AlertManagerTest, ClearAllManualAlerts) {
  auto& alerts = AlertManager::GetInstance();

  alerts.Info("Info1", "Info 1");
  alerts.Warning("Warn1", "Warning 1");
  alerts.Error("Error1", "Error 1");

  alerts.ClearAllManualAlerts();

  // All should be removed
  EXPECT_FALSE(alerts.ClearManualAlert("Info1"));
  EXPECT_FALSE(alerts.ClearManualAlert("Warn1"));
  EXPECT_FALSE(alerts.ClearManualAlert("Error1"));
}

TEST_F(AlertManagerTest, ClearAllAutoAlerts) {
  auto& alerts = AlertManager::GetInstance();

  alerts.AddThresholdAlert("/Test/A", "Alert A", 10.0,
                  AlertCondition::kAbove, frc::Alert::AlertType::kWarning);
  alerts.AddThresholdAlert("/Test/B", "Alert B", 5.0,
                  AlertCondition::kBelow, frc::Alert::AlertType::kWarning);

  alerts.ClearAllAutoAlerts();

  // All should be removed
  EXPECT_FALSE(alerts.ClearAutoAlert("/Test/A"));
  EXPECT_FALSE(alerts.ClearAutoAlert("/Test/B"));
}

TEST_F(AlertManagerTest, ClearAll) {
  auto& alerts = AlertManager::GetInstance();

  // Add manual alerts
  alerts.Info("Info1", "Info 1");
  alerts.Warning("Warn1", "Warning 1");

  // Add auto alerts
  alerts.AddThresholdAlert("/Test/A", "Alert A", 10.0,
                  AlertCondition::kAbove, frc::Alert::AlertType::kWarning);

  alerts.Clear();

  // All should be removed
  EXPECT_FALSE(alerts.ClearManualAlert("Info1"));
  EXPECT_FALSE(alerts.ClearManualAlert("Warn1"));
  EXPECT_FALSE(alerts.ClearAutoAlert("/Test/A"));
}

TEST_F(AlertManagerTest, ClearNonExistentAlert) {
  auto& alerts = AlertManager::GetInstance();

  bool removed = alerts.ClearManualAlert("NonExistent");
  EXPECT_FALSE(removed);

  removed = alerts.ClearAutoAlert("/NonExistent/Key");
  EXPECT_FALSE(removed);
}

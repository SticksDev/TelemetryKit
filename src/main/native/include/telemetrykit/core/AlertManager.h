#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <frc/Alert.h>

namespace tkit {

/**
 * Comparison rule for auto-checked alerts.
 */
enum class AlertCondition {
  kAbove,       ///< Triggers when value > threshold
  kBelow,       ///< Triggers when value < threshold
  kOutOfRange,  ///< Triggers when value is outside [min, max]
  kNotEqual     ///< Triggers when value != expected (used for change/state checks)
};

/**
 * Central place to create and manage Driver Station alerts (WPILib frc::Alert).
 * 
 * Supports both manual alerts (turned on/off by user code) and
 * auto-checked alerts that monitor logged values against thresholds.
 * 
 * See the documentation for each method for usage details.
 */
class AlertManager {
 public:
  /// Returns the singleton instance.
  static AlertManager& GetInstance();

  // === Auto-checked alerts ===

  /**
   * Adds an alert that watches a logged value and compares it to a threshold.
   *
   * @param logKey Path/key in the log table (ex: "/Power/Battery")
   * @param alertText Message shown on the Driver Station
   * @param threshold Value to compare against
   * @param condition kAbove or kBelow
   * @param type Alert severity (Info/Warning/Error)
   */
  void AddThresholdAlert(std::string_view logKey,
                         std::string_view alertText,
                         double threshold,
                         AlertCondition condition,
                         frc::Alert::AlertType type = frc::Alert::AlertType::kWarning);

  /**
   * Adds an alert that triggers when the value leaves the allowed range [min, max].
   *
   * @param logKey Path/key in the log table
   * @param alertText Message shown on the Driver Station
   * @param min Lowest allowed value
   * @param max Highest allowed value
   * @param type Alert severity
   */
  void AddRangeAlert(std::string_view logKey,
                     std::string_view alertText,
                     double min,
                     double max,
                     frc::Alert::AlertType type = frc::Alert::AlertType::kWarning);

  /**
   * Adds an alert that triggers when a numeric value differs from what we expect.
   * Handy for catching unexpected mode/state changes.
   *
   * @param logKey Path/key in the log table
   * @param alertText Message shown on the Driver Station
   * @param expectedValue Value we expect to see
   * @param type Alert severity
   */
  void AddOnChangeAlert(std::string_view logKey,
                        std::string_view alertText,
                        double expectedValue,
                        frc::Alert::AlertType type = frc::Alert::AlertType::kWarning);

  /**
   * Same as above, but for boolean values.
   *
   * @param logKey Path/key in the log table
   * @param alertText Message shown on the Driver Station
   * @param expectedValue Value we expect to see
   * @param type Alert severity
   */
  void AddOnChangeAlert(std::string_view logKey,
                        std::string_view alertText,
                        bool expectedValue,
                        frc::Alert::AlertType type = frc::Alert::AlertType::kWarning);

  /**
   * Adds an alert that triggers when a string value differs from what we expect.
   *
   * @param logKey Path/key in the log table
   * @param alertText Message shown on the Driver Station
   * @param expectedValue String we expect to see
   * @param type Alert severity
   */
  void AddOnChangeAlert(std::string_view logKey,
                        std::string_view alertText,
                        std::string_view expectedValue,
                        frc::Alert::AlertType type = frc::Alert::AlertType::kWarning);

  /**
   * Adds an alert that triggers when a double array differs from what we expect.
   *
   * @param logKey Path/key in the log table
   * @param alertText Message shown on the Driver Station
   * @param expectedValue Array we expect to see
   * @param type Alert severity
   */
  void AddOnChangeAlert(std::string_view logKey,
                        std::string_view alertText,
                        const std::vector<double>& expectedValue,
                        frc::Alert::AlertType type = frc::Alert::AlertType::kWarning);

  /**
   * Adds an alert that triggers when a string array differs from what we expect.
   *
   * @param logKey Path/key in the log table
   * @param alertText Message shown on the Driver Station
   * @param expectedValue Array we expect to see
   * @param type Alert severity
   */
  void AddOnChangeAlert(std::string_view logKey,
                        std::string_view alertText,
                        const std::vector<std::string>& expectedValue,
                        frc::Alert::AlertType type = frc::Alert::AlertType::kWarning);

  // === Manual alerts ===

  /**
   * Turns an info alert on/off.
   *
   * @param key Unique ID for this alert
   * @param text Message shown on the Driver Station
   * @param active true = show, false = clear
   */
  void Info(std::string_view key, std::string_view text, bool active = true);

  /**
   * Turns a warning alert on/off.
   *
   * @param key Unique ID for this alert
   * @param text Message shown on the Driver Station
   * @param active true = show, false = clear
   */
  void Warning(std::string_view key, std::string_view text, bool active = true);

  /**
   * Turns an error alert on/off.
   *
   * @param key Unique ID for this alert
   * @param text Message shown on the Driver Station
   * @param active true = show, false = clear
   */
  void Error(std::string_view key, std::string_view text, bool active = true);

  // === Clearing ===

  /**
   * Removes a manual alert completely.
   *
   * @param key Key used to create the alert
   * @return true if an alert existed and was removed
   */
  bool ClearManualAlert(std::string_view key);

  /**
   * Removes auto-checked alerts that match a log key.
   *
   * @param logKey Log key used when adding the alert(s)
   * @return true if at least one alert was removed
   */
  bool ClearAutoAlert(std::string_view logKey);

  /// Removes all manual alerts.
  void ClearAllManualAlerts();

  /// Removes all auto-checked alerts.
  void ClearAllAutoAlerts();

  /// Removes everything (manual + auto-checked).
  void Clear();

  // === Internal ===

  /**
   * Evaluates all auto-checked alerts using the latest logged values.
   * Typically called from your periodic logging loop.
   */
  void CheckAll();

 private:
  AlertManager() = default;
  ~AlertManager() = default;
  AlertManager(const AlertManager&) = delete;
  AlertManager& operator=(const AlertManager&) = delete;

  enum class ExpectedType {
    kNumeric,
    kBool,
    kString,
    kDoubleArray,
    kStringArray
  };

  struct AlertBinding {
    std::string logKey;
    AlertCondition condition;
    double threshold{0.0};
    double rangeMin{0.0};
    double rangeMax{0.0};
    double expectedValue{0.0};                      // For kNotEqual numeric
    bool expectedBool{false};                       // For kNotEqual bool
    std::string expectedString;                     // For kNotEqual string
    std::vector<double> expectedDoubleArray;        // For kNotEqual double array
    std::vector<std::string> expectedStringArray;   // For kNotEqual string array
    ExpectedType expectedType{ExpectedType::kNumeric};
    std::unique_ptr<frc::Alert> alert;
  };

  frc::Alert& GetOrCreateAlert(std::string_view key,
                               std::string_view text,
                               frc::Alert::AlertType type);

  std::vector<AlertBinding> m_autoAlerts;
  std::unordered_map<std::string, std::unique_ptr<frc::Alert>> m_manualAlerts;
  mutable std::mutex m_mutex;
};

}  // namespace tkit

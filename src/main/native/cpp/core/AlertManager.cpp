#include "telemetrykit/core/AlertManager.h"

#include <algorithm>

#include <fmt/format.h>
#include <frc/Errors.h>

#include "telemetrykit/core/Logger.h"

namespace tkit {

AlertManager& AlertManager::GetInstance() {
  static AlertManager instance;
  return instance;
}

void AlertManager::AddThresholdAlert(std::string_view logKey,
                                     std::string_view alertText,
                                     double threshold,
                                     AlertCondition condition,
                                     frc::Alert::AlertType type) {
  std::lock_guard<std::mutex> lock(m_mutex);

  AlertBinding binding;
  binding.logKey = std::string(logKey);
  binding.condition = condition;
  binding.threshold = threshold;
  binding.alert = std::make_unique<frc::Alert>(
      "TelemetryKit", std::string(alertText), type);

  m_autoAlerts.push_back(std::move(binding));
}

void AlertManager::AddRangeAlert(std::string_view logKey,
                                 std::string_view alertText,
                                 double min,
                                 double max,
                                 frc::Alert::AlertType type) {
  std::lock_guard<std::mutex> lock(m_mutex);

  AlertBinding binding;
  binding.logKey = std::string(logKey);
  binding.condition = AlertCondition::kOutOfRange;
  binding.rangeMin = min;
  binding.rangeMax = max;
  binding.alert = std::make_unique<frc::Alert>(
      "TelemetryKit", std::string(alertText), type);

  m_autoAlerts.push_back(std::move(binding));
}

void AlertManager::AddOnChangeAlert(std::string_view logKey,
                                    std::string_view alertText,
                                    double expectedValue,
                                    frc::Alert::AlertType type) {
  std::lock_guard<std::mutex> lock(m_mutex);

  AlertBinding binding;
  binding.logKey = std::string(logKey);
  binding.condition = AlertCondition::kNotEqual;
  binding.expectedValue = expectedValue;
  binding.expectedType = ExpectedType::kNumeric;
  binding.alert = std::make_unique<frc::Alert>(
      "TelemetryKit", std::string(alertText), type);

  m_autoAlerts.push_back(std::move(binding));
}

void AlertManager::AddOnChangeAlert(std::string_view logKey,
                                    std::string_view alertText,
                                    bool expectedValue,
                                    frc::Alert::AlertType type) {
  std::lock_guard<std::mutex> lock(m_mutex);

  AlertBinding binding;
  binding.logKey = std::string(logKey);
  binding.condition = AlertCondition::kNotEqual;
  binding.expectedBool = expectedValue;
  binding.expectedType = ExpectedType::kBool;
  binding.alert = std::make_unique<frc::Alert>(
      "TelemetryKit", std::string(alertText), type);

  m_autoAlerts.push_back(std::move(binding));
}

void AlertManager::AddOnChangeAlert(std::string_view logKey,
                                    std::string_view alertText,
                                    std::string_view expectedValue,
                                    frc::Alert::AlertType type) {
  std::lock_guard<std::mutex> lock(m_mutex);

  AlertBinding binding;
  binding.logKey = std::string(logKey);
  binding.condition = AlertCondition::kNotEqual;
  binding.expectedString = std::string(expectedValue);
  binding.expectedType = ExpectedType::kString;
  binding.alert = std::make_unique<frc::Alert>(
      "TelemetryKit", std::string(alertText), type);

  m_autoAlerts.push_back(std::move(binding));
}

void AlertManager::AddOnChangeAlert(std::string_view logKey,
                                    std::string_view alertText,
                                    const std::vector<double>& expectedValue,
                                    frc::Alert::AlertType type) {
  std::lock_guard<std::mutex> lock(m_mutex);

  AlertBinding binding;
  binding.logKey = std::string(logKey);
  binding.condition = AlertCondition::kNotEqual;
  binding.expectedDoubleArray = expectedValue;
  binding.expectedType = ExpectedType::kDoubleArray;
  binding.alert = std::make_unique<frc::Alert>(
      "TelemetryKit", std::string(alertText), type);

  m_autoAlerts.push_back(std::move(binding));
}

void AlertManager::AddOnChangeAlert(std::string_view logKey,
                                    std::string_view alertText,
                                    const std::vector<std::string>& expectedValue,
                                    frc::Alert::AlertType type) {
  std::lock_guard<std::mutex> lock(m_mutex);

  AlertBinding binding;
  binding.logKey = std::string(logKey);
  binding.condition = AlertCondition::kNotEqual;
  binding.expectedStringArray = expectedValue;
  binding.expectedType = ExpectedType::kStringArray;
  binding.alert = std::make_unique<frc::Alert>(
      "TelemetryKit", std::string(alertText), type);

  m_autoAlerts.push_back(std::move(binding));
}

frc::Alert& AlertManager::GetOrCreateAlert(std::string_view key,
                                           std::string_view text,
                                           frc::Alert::AlertType type) {
  std::string keyStr(key);
  auto it = m_manualAlerts.find(keyStr);
  if (it == m_manualAlerts.end()) {
    auto alert = std::make_unique<frc::Alert>(
        "TelemetryKit", std::string(text), type);
    auto [inserted, success] = m_manualAlerts.emplace(keyStr, std::move(alert));
    return *inserted->second;
  }
  // Update text if it changed
  it->second->SetText(std::string(text));
  return *it->second;
}

void AlertManager::Info(std::string_view key, std::string_view text, bool active) {
  std::lock_guard<std::mutex> lock(m_mutex);
  GetOrCreateAlert(key, text, frc::Alert::AlertType::kInfo).Set(active);
}

void AlertManager::Warning(std::string_view key, std::string_view text, bool active) {
  std::lock_guard<std::mutex> lock(m_mutex);
  GetOrCreateAlert(key, text, frc::Alert::AlertType::kWarning).Set(active);
}

void AlertManager::Error(std::string_view key, std::string_view text, bool active) {
  std::lock_guard<std::mutex> lock(m_mutex);
  GetOrCreateAlert(key, text, frc::Alert::AlertType::kError).Set(active);
}

void AlertManager::CheckAll() {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto& rootTable = Logger::GetInstance().GetRootTable();

  for (auto& binding : m_autoAlerts) {
    auto value = rootTable.Get(binding.logKey);
    if (!value.has_value()) {
      continue;  // Key not logged yet
    }

    bool active = false;

    // Handle kNotEqual with non-numeric expected types
    if (binding.condition == AlertCondition::kNotEqual) {
      switch (binding.expectedType) {
        case ExpectedType::kBool:
          if (value->Is<bool>()) {
            active = value->Get<bool>() != binding.expectedBool;
          }
          binding.alert->Set(active);
          continue;

        case ExpectedType::kString:
          if (value->Is<std::string>()) {
            active = value->Get<std::string>() != binding.expectedString;
          }
          binding.alert->Set(active);
          continue;

        case ExpectedType::kDoubleArray:
          if (value->Is<std::vector<double>>()) {
            active = value->Get<std::vector<double>>() != binding.expectedDoubleArray;
          }
          binding.alert->Set(active);
          continue;

        case ExpectedType::kStringArray:
          if (value->Is<std::vector<std::string>>()) {
            active = value->Get<std::vector<std::string>>() != binding.expectedStringArray;
          }
          binding.alert->Set(active);
          continue;

        case ExpectedType::kNumeric:
          // Fall through to numeric handling below
          break;
      }
    }

    // Extract numeric value
    double numericValue = 0.0;
    bool supported = true;

    switch (value->GetType()) {
      case LogType::kDouble:
        numericValue = value->Get<double>();
        break;
      case LogType::kFloat:
        numericValue = static_cast<double>(value->Get<float>());
        break;
      case LogType::kInt64:
        numericValue = static_cast<double>(value->Get<int64_t>());
        break;
      case LogType::kBoolean:
        numericValue = value->Get<bool>() ? 1.0 : 0.0;
        break;
      default:
        FRC_ReportError(frc::err::Error, "{}",
            fmt::format("TelemetryKit: AlertManager: Unsupported log value type for alert on key '{}'", binding.logKey));
        supported = false;
        break;
    }

    if (!supported) {
      continue;
    }

    switch (binding.condition) {
      case AlertCondition::kAbove:
        active = numericValue > binding.threshold;
        break;
      case AlertCondition::kBelow:
        active = numericValue < binding.threshold;
        break;
      case AlertCondition::kOutOfRange:
        active = numericValue < binding.rangeMin || numericValue > binding.rangeMax;
        break;
      case AlertCondition::kNotEqual:
        active = numericValue != binding.expectedValue;
        break;
    }

    binding.alert->Set(active);
  }
}

bool AlertManager::ClearManualAlert(std::string_view key) {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_manualAlerts.erase(std::string(key)) > 0;
}

bool AlertManager::ClearAutoAlert(std::string_view logKey) {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string keyStr(logKey);

  auto it = std::remove_if(m_autoAlerts.begin(), m_autoAlerts.end(),
      [&keyStr](const AlertBinding& binding) {
        return binding.logKey == keyStr;
      });

  if (it != m_autoAlerts.end()) {
    m_autoAlerts.erase(it, m_autoAlerts.end());
    return true;
  }
  return false;
}

void AlertManager::ClearAllManualAlerts() {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_manualAlerts.clear();
}

void AlertManager::ClearAllAutoAlerts() {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_autoAlerts.clear();
}

void AlertManager::Clear() {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_autoAlerts.clear();
  m_manualAlerts.clear();
}

}  // namespace tkit

#include "telemetrykit/receiver/ConsoleReceiver.h"

#include <iomanip>
#include <sstream>

#include "telemetrykit/core/LogTable.h"

namespace telemetrykit {

ConsoleReceiver::ConsoleReceiver(std::string_view prefix, bool printOnlyChanges)
  : m_prefix(prefix), m_printOnlyChanges(printOnlyChanges), m_output(std::cout) {}

void ConsoleReceiver::OnStart() {
  m_lastValues.clear();
  m_output << m_prefix << " Logging started" << std::endl;
}

void ConsoleReceiver::OnUpdate(const LogTable& table, int64_t timestamp) {
  // Get all entries from the table
  auto entries = table.GetAllEntries();

  // Process each entry
  for (const auto& [key, value] : entries) {
    // Check if value has changed
    bool hasChanged = true;
    auto lastIt = m_lastValues.find(key);
    if (lastIt != m_lastValues.end()) {
      hasChanged = (lastIt->second != value);
    }

    // Skip if unchanged and we only want changes
    if (m_printOnlyChanges && !hasChanged) {
      continue;
    }

    // Print the value
    m_output << m_prefix << " "
             << std::setw(40) << std::left << key << " = "
             << FormatValue(value);

    // Add timestamp if changed
    if (hasChanged) {
      m_output << " (t=" << timestamp << ")";
    }

    m_output << std::endl;

    // Update last value
    m_lastValues[key] = value;
  }
}

void ConsoleReceiver::OnEnd() {
  m_output << m_prefix << " Logging ended" << std::endl;
  m_lastValues.clear();
}

std::string ConsoleReceiver::FormatValue(const LogValue& value) const {
  std::stringstream ss;

  switch (value.GetType()) {
    case LogType::kBoolean:
      ss << (value.Get<bool>() ? "true" : "false");
      break;

    case LogType::kInt64:
      ss << value.Get<int64_t>();
      break;

    case LogType::kFloat:
      ss << std::fixed << std::setprecision(3) << value.Get<float>();
      break;

    case LogType::kDouble:
      ss << std::fixed << std::setprecision(3) << value.Get<double>();
      break;

    case LogType::kString:
      ss << "\"" << value.Get<std::string>() << "\"";
      break;

    case LogType::kBooleanArray: {
      const auto& arr = value.Get<std::vector<bool>>();
      ss << "[";
      for (size_t i = 0; i < arr.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << (arr[i] ? "true" : "false");
        if (i >= 9 && arr.size() > 10) {
          ss << ", ... (" << arr.size() << " total)";
          break;
        }
      }
      ss << "]";
      break;
    }

    case LogType::kInt64Array: {
      const auto& arr = value.Get<std::vector<int64_t>>();
      ss << "[";
      for (size_t i = 0; i < arr.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << arr[i];
        if (i >= 9 && arr.size() > 10) {
          ss << ", ... (" << arr.size() << " total)";
          break;
        }
      }
      ss << "]";
      break;
    }

    case LogType::kFloatArray: {
      const auto& arr = value.Get<std::vector<float>>();
      ss << "[";
      for (size_t i = 0; i < arr.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << std::fixed << std::setprecision(3) << arr[i];
        if (i >= 9 && arr.size() > 10) {
          ss << ", ... (" << arr.size() << " total)";
          break;
        }
      }
      ss << "]";
      break;
    }

    case LogType::kDoubleArray: {
      const auto& arr = value.Get<std::vector<double>>();
      ss << "[";
      for (size_t i = 0; i < arr.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << std::fixed << std::setprecision(3) << arr[i];
        if (i >= 9 && arr.size() > 10) {
          ss << ", ... (" << arr.size() << " total)";
          break;
        }
      }
      ss << "]";
      break;
    }

    case LogType::kStringArray: {
      const auto& arr = value.Get<std::vector<std::string>>();
      ss << "[";
      for (size_t i = 0; i < arr.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "\"" << arr[i] << "\"";
        if (i >= 4 && arr.size() > 5) {
          ss << ", ... (" << arr.size() << " total)";
          break;
        }
      }
      ss << "]";
      break;
    }

    case LogType::kRaw: {
      const auto& data = value.Get<std::vector<uint8_t>>();
      ss << "<raw: " << data.size() << " bytes>";
      break;
    }

    case LogType::kStruct: {
      const auto& data = value.Get<std::vector<uint8_t>>();
      ss << "<struct " << value.GetTypeString() << ": " << data.size() << " bytes>";
      break;
    }

    case LogType::kStructArray: {
      const auto& data = value.Get<std::vector<uint8_t>>();
      ss << "<struct[] " << value.GetTypeString() << ": " << data.size() << " bytes>";
      break;
    }
  }

  return ss.str();
}

}  // namespace telemetrykit

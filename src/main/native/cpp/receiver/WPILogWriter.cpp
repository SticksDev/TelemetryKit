#include "telemetrykit/receiver/WPILogWriter.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

#include "telemetrykit/core/LogTable.h"

namespace tkit {

WPILogWriter::WPILogWriter(std::string_view logPath)
  : m_logPath(logPath) {}

void WPILogWriter::OnStart() {
  // Create log directory if it doesn't exist
  std::filesystem::create_directories(m_logPath);

  // Generate log file name with timestamp
  std::string logFileName = GenerateLogFileName();
  std::string fullPath = m_logPath + "/" + logFileName;

  // Create DataLogWriter
  std::error_code ec;
  m_log = std::make_unique<wpi::log::DataLogWriter>(fullPath, ec);

  // Clear state
  m_entryIds.clear();
  m_lastValues.clear();
}

void WPILogWriter::OnUpdate(const LogTable& table, int64_t timestamp) {
  if (!m_log) {
    return;
  }

  // Get all entries from the table
  auto entries = table.GetAllEntries();

  // Process each entry
  for (const auto& [key, value] : entries) {
    // Check if value has changed (field-change-only optimization)
    auto lastIt = m_lastValues.find(key);
    if (lastIt != m_lastValues.end() && lastIt->second == value) {
      continue;  // No change, skip
    }

    // Get or create entry
    int entryId = GetOrCreateEntry(key, value);

    // Append value
    AppendValue(entryId, value, timestamp);

    // Update last value
    m_lastValues[key] = value;
  }

  // Flush to ensure data is written
  m_log->Flush();
}

void WPILogWriter::OnEnd() {
  // Reset the log writer (destructor will handle flushing)
  m_log.reset();

  m_entryIds.clear();
  m_lastValues.clear();
}

int WPILogWriter::GetOrCreateEntry(std::string_view key, const LogValue& value) {
  // Check if entry already exists
  std::string keyStr(key);
  auto it = m_entryIds.find(keyStr);
  if (it != m_entryIds.end()) {
    return it->second;
  }

  // Create new entry based on type
  int entryId = 0;

  switch (value.GetType()) {
    case LogType::kBoolean:
      entryId = m_log->Start(keyStr, "boolean");
      break;
    case LogType::kInt64:
      entryId = m_log->Start(keyStr, "int64");
      break;
    case LogType::kFloat:
      entryId = m_log->Start(keyStr, "float");
      break;
    case LogType::kDouble:
      entryId = m_log->Start(keyStr, "double");
      break;
    case LogType::kString:
      entryId = m_log->Start(keyStr, "string");
      break;
    case LogType::kBooleanArray:
      entryId = m_log->Start(keyStr, "boolean[]");
      break;
    case LogType::kInt64Array:
      entryId = m_log->Start(keyStr, "int64[]");
      break;
    case LogType::kFloatArray:
      entryId = m_log->Start(keyStr, "float[]");
      break;
    case LogType::kDoubleArray:
      entryId = m_log->Start(keyStr, "double[]");
      break;
    case LogType::kStringArray:
      entryId = m_log->Start(keyStr, "string[]");
      break;
    case LogType::kRaw:
      entryId = m_log->Start(keyStr, "raw");
      break;
    case LogType::kStruct: {
      // For structs, use "struct:TypeName" format (e.g., "struct:Pose2d")
      std::string typeStr = "struct:" + value.GetTypeString();
      entryId = m_log->Start(keyStr, typeStr);
      break;
    }
    case LogType::kStructArray: {
      // For struct arrays, use "struct:TypeName[]" format (e.g., "struct:Pose2d[]")
      std::string typeStr = "struct:" + value.GetTypeString() + "[]";
      entryId = m_log->Start(keyStr, typeStr);
      break;
    }
  }

  // Cache the entry ID
  m_entryIds[keyStr] = entryId;

  return entryId;
}

void WPILogWriter::AppendValue(int entryId, const LogValue& value, int64_t timestamp) {
  switch (value.GetType()) {
    case LogType::kBoolean:
      m_log->AppendBoolean(entryId, value.Get<bool>(), timestamp);
      break;

    case LogType::kInt64:
      m_log->AppendInteger(entryId, value.Get<int64_t>(), timestamp);
      break;

    case LogType::kFloat:
      m_log->AppendFloat(entryId, value.Get<float>(), timestamp);
      break;

    case LogType::kDouble:
      m_log->AppendDouble(entryId, value.Get<double>(), timestamp);
      break;

    case LogType::kString:
      m_log->AppendString(entryId, value.Get<std::string>(), timestamp);
      break;

    case LogType::kBooleanArray: {
      const auto& arr = value.Get<std::vector<bool>>();
      // Convert vector<bool> to vector<int> for WPILib
      std::vector<int> intArr;
      intArr.reserve(arr.size());
      for (bool b : arr) {
        intArr.push_back(b ? 1 : 0);
      }
      m_log->AppendBooleanArray(entryId, intArr, timestamp);
      break;
    }

    case LogType::kInt64Array:
      m_log->AppendIntegerArray(entryId, value.Get<std::vector<int64_t>>(), timestamp);
      break;

    case LogType::kFloatArray:
      m_log->AppendFloatArray(entryId, value.Get<std::vector<float>>(), timestamp);
      break;

    case LogType::kDoubleArray:
      m_log->AppendDoubleArray(entryId, value.Get<std::vector<double>>(), timestamp);
      break;

    case LogType::kStringArray:
      m_log->AppendStringArray(entryId, value.Get<std::vector<std::string>>(), timestamp);
      break;

    case LogType::kRaw:
    case LogType::kStruct:
    case LogType::kStructArray: {
      // Raw and struct data stored as raw bytes
      const auto& data = value.Get<std::vector<uint8_t>>();
      m_log->AppendRaw(entryId, data, timestamp);
      break;
    }
  }
}

std::string WPILogWriter::GenerateLogFileName() const {
  // Get current time
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);

  // Format: TelemetryKit_YYYYMMDD_HHMMSS.wpilog
  std::stringstream ss;
  ss << "TelemetryKit_"
     << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S")
     << ".wpilog";

  return ss.str();
}

}  // namespace tkit

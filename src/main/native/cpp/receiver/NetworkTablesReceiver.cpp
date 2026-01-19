#include "telemetrykit/receiver/NetworkTablesReceiver.h"

#include "telemetrykit/core/LogTable.h"

namespace tkit {

NetworkTablesReceiver::NetworkTablesReceiver()
  : m_inst(nt::NetworkTableInstance::GetDefault()),
    m_prefix(kDefaultPrefix) {}

NetworkTablesReceiver::NetworkTablesReceiver(nt::NetworkTableInstance inst)
  : m_inst(inst),
    m_prefix(kDefaultPrefix) {}

NetworkTablesReceiver::NetworkTablesReceiver(nt::NetworkTableInstance inst, std::string_view prefix)
  : m_inst(inst),
    m_prefix(prefix) {}

void NetworkTablesReceiver::OnStart() {
  // Clear state
  m_publishers.clear();
  m_lastValues.clear();
}

void NetworkTablesReceiver::OnUpdate(const LogTable& table, int64_t timestamp) {
  // Get all entries from the table
  auto entries = table.GetAllEntries();

  // Process each entry
  for (const auto& [key, value] : entries) {
    // Check if value has changed (field-change-only optimization)
    auto lastIt = m_lastValues.find(key);
    if (lastIt != m_lastValues.end() && lastIt->second == value) {
      continue;  // No change, skip
    }

    // Publish value
    PublishValue(key, value, timestamp);

    // Update last value
    m_lastValues[key] = value;
  }

  // Flush NetworkTables
  m_inst.Flush();
}

void NetworkTablesReceiver::OnEnd() {
  // Publishers are auto-closed by destructors
  m_publishers.clear();
  m_lastValues.clear();
}

void NetworkTablesReceiver::PublishValue(std::string_view key, const LogValue& value, int64_t timestamp) {
  // Build full key with prefix (e.g., "/TelemetryKit/Drive/Speed")
  std::string keyStr = m_prefix + std::string(key);

  // Check if we already have a publisher for this key
  auto pubIt = m_publishers.find(keyStr);

  switch (value.GetType()) {
    case LogType::kBoolean: {
      if (pubIt == m_publishers.end()) {
        auto topic = m_inst.GetBooleanTopic(keyStr);
        m_publishers[keyStr] = topic.Publish();
        pubIt = m_publishers.find(keyStr);
      }
      std::get<nt::BooleanPublisher>(pubIt->second).Set(value.Get<bool>(), timestamp);
      break;
    }

    case LogType::kInt64: {
      if (pubIt == m_publishers.end()) {
        auto topic = m_inst.GetIntegerTopic(keyStr);
        m_publishers[keyStr] = topic.Publish();
        pubIt = m_publishers.find(keyStr);
      }
      std::get<nt::IntegerPublisher>(pubIt->second).Set(value.Get<int64_t>(), timestamp);
      break;
    }

    case LogType::kFloat: {
      if (pubIt == m_publishers.end()) {
        auto topic = m_inst.GetFloatTopic(keyStr);
        m_publishers[keyStr] = topic.Publish();
        pubIt = m_publishers.find(keyStr);
      }
      std::get<nt::FloatPublisher>(pubIt->second).Set(value.Get<float>(), timestamp);
      break;
    }

    case LogType::kDouble: {
      if (pubIt == m_publishers.end()) {
        auto topic = m_inst.GetDoubleTopic(keyStr);
        m_publishers[keyStr] = topic.Publish();
        pubIt = m_publishers.find(keyStr);
      }
      std::get<nt::DoublePublisher>(pubIt->second).Set(value.Get<double>(), timestamp);
      break;
    }

    case LogType::kString: {
      if (pubIt == m_publishers.end()) {
        auto topic = m_inst.GetStringTopic(keyStr);
        m_publishers[keyStr] = topic.Publish();
        pubIt = m_publishers.find(keyStr);
      }
      std::get<nt::StringPublisher>(pubIt->second).Set(value.Get<std::string>(), timestamp);
      break;
    }

    case LogType::kBooleanArray: {
      if (pubIt == m_publishers.end()) {
        auto topic = m_inst.GetBooleanArrayTopic(keyStr);
        m_publishers[keyStr] = topic.Publish();
        pubIt = m_publishers.find(keyStr);
      }
      const auto& arr = value.Get<std::vector<bool>>();
      // Convert vector<bool> to vector<int> for NetworkTables
      std::vector<int> intArr;
      intArr.reserve(arr.size());
      for (bool b : arr) {
        intArr.push_back(b ? 1 : 0);
      }
      std::get<nt::BooleanArrayPublisher>(pubIt->second).Set(intArr, timestamp);
      break;
    }

    case LogType::kInt64Array: {
      if (pubIt == m_publishers.end()) {
        auto topic = m_inst.GetIntegerArrayTopic(keyStr);
        m_publishers[keyStr] = topic.Publish();
        pubIt = m_publishers.find(keyStr);
      }
      std::get<nt::IntegerArrayPublisher>(pubIt->second).Set(value.Get<std::vector<int64_t>>(), timestamp);
      break;
    }

    case LogType::kFloatArray: {
      if (pubIt == m_publishers.end()) {
        auto topic = m_inst.GetFloatArrayTopic(keyStr);
        m_publishers[keyStr] = topic.Publish();
        pubIt = m_publishers.find(keyStr);
      }
      std::get<nt::FloatArrayPublisher>(pubIt->second).Set(value.Get<std::vector<float>>(), timestamp);
      break;
    }

    case LogType::kDoubleArray: {
      if (pubIt == m_publishers.end()) {
        auto topic = m_inst.GetDoubleArrayTopic(keyStr);
        m_publishers[keyStr] = topic.Publish();
        pubIt = m_publishers.find(keyStr);
      }
      std::get<nt::DoubleArrayPublisher>(pubIt->second).Set(value.Get<std::vector<double>>(), timestamp);
      break;
    }

    case LogType::kStringArray: {
      if (pubIt == m_publishers.end()) {
        auto topic = m_inst.GetStringArrayTopic(keyStr);
        m_publishers[keyStr] = topic.Publish();
        pubIt = m_publishers.find(keyStr);
      }
      std::get<nt::StringArrayPublisher>(pubIt->second).Set(value.Get<std::vector<std::string>>(), timestamp);
      break;
    }

    case LogType::kRaw: {
      if (pubIt == m_publishers.end()) {
        auto topic = m_inst.GetRawTopic(keyStr);
        m_publishers[keyStr] = topic.Publish("raw");
        pubIt = m_publishers.find(keyStr);
      }
      std::get<nt::RawPublisher>(pubIt->second).Set(value.Get<std::vector<uint8_t>>(), timestamp);
      break;
    }

    case LogType::kStruct:
    case LogType::kStructArray: {
      // For structs, publish as RawTopic with type string metadata.
      // NT4 clients like AdvantageScope will use the type string and schema to deserialize.
      if (pubIt == m_publishers.end()) {
        std::string typeStr = value.GetTypeString();  // e.g., "struct:Pose2d"

        // Register struct schema with NT (handles nested schemas automatically)
        value.RegisterSchema(m_inst);

        // Create RawTopic publisher with type string
        auto topic = m_inst.GetRawTopic(keyStr);
        m_publishers[keyStr] = topic.Publish(typeStr);
        pubIt = m_publishers.find(keyStr);
      }
      std::get<nt::RawPublisher>(pubIt->second).Set(value.Get<std::vector<uint8_t>>(), timestamp);
      break;
    }
  }
}

}  // namespace tkit

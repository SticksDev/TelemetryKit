#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include <networktables/BooleanArrayTopic.h>
#include <networktables/BooleanTopic.h>
#include <networktables/DoubleArrayTopic.h>
#include <networktables/DoubleTopic.h>
#include <networktables/FloatArrayTopic.h>
#include <networktables/FloatTopic.h>
#include <networktables/IntegerArrayTopic.h>
#include <networktables/IntegerTopic.h>
#include <networktables/NetworkTableInstance.h>
#include <networktables/RawTopic.h>
#include <networktables/StringArrayTopic.h>
#include <networktables/StringTopic.h>

#include "telemetrykit/receiver/LogDataReceiver.h"
#include "telemetrykit/core/LogValue.h"

namespace telemetrykit {

/**
 * NetworkTablesReceiver - Publishes telemetry data to NetworkTables 4.
 *
 * Publishes all logged values to NetworkTables for real-time monitoring.
 * Compatible with dashboards like Shuffleboard, Glass, and AdvantageScope.
 *
 * Example Usage:
 *
 *   auto& logger = Logger::GetInstance();
 *   logger.AddReceiver(
 *     std::make_unique<NetworkTablesReceiver>()
 *   );
 */
class NetworkTablesReceiver : public LogDataReceiver {
 public:
  /**
   * Create a NetworkTablesReceiver using the default instance.
   */
  NetworkTablesReceiver();

  /**
   * Create a NetworkTablesReceiver with a custom NT instance.
   *
   * @param inst The NetworkTables instance to use
   */
  explicit NetworkTablesReceiver(nt::NetworkTableInstance inst);

  // LogDataReceiver interface
  void OnStart() override;
  void OnUpdate(const LogTable& table, int64_t timestamp) override;
  void OnEnd() override;

 private:
  /**
   * Variant type to hold different publisher types.
   */
  using PublisherVariant = std::variant<
    std::monostate,
    nt::BooleanPublisher,
    nt::IntegerPublisher,
    nt::FloatPublisher,
    nt::DoublePublisher,
    nt::StringPublisher,
    nt::BooleanArrayPublisher,
    nt::IntegerArrayPublisher,
    nt::FloatArrayPublisher,
    nt::DoubleArrayPublisher,
    nt::StringArrayPublisher,
    nt::RawPublisher
  >;

  /**
   * Publish a value to NetworkTables.
   *
   * @param key The key (e.g., "/Drivetrain/Speed")
   * @param value The value to publish
   * @param timestamp Timestamp in microseconds
   */
  void PublishValue(std::string_view key, const LogValue& value, int64_t timestamp);

  nt::NetworkTableInstance m_inst;
  std::unordered_map<std::string, PublisherVariant> m_publishers;
  std::unordered_map<std::string, LogValue> m_lastValues;  // For change detection
  std::unordered_set<std::string> m_publishedSchemas;  // Track published schemas to avoid duplicates
};

}  // namespace telemetrykit

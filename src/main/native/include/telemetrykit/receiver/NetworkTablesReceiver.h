#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
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

namespace tkit {

/**
 * Receiver that writes logged telemetry to NetworkTables (NT4).
 *
 * All keys are published under a root prefix (default: "/TelemetryKit").
 * For example, "/Drive/Speed" becomes "/TelemetryKit/Drive/Speed".
 */
class NetworkTablesReceiver : public LogDataReceiver {
 public:
  /// Default root path used for publishing.
  static constexpr std::string_view kDefaultPrefix = "/TelemetryKit";

  /**
   * Uses the default NetworkTables instance and the default prefix.
   */
  NetworkTablesReceiver();

  /**
   * Uses a specific NetworkTables instance and the default prefix.
   *
   * @param inst NetworkTables instance to publish through.
   */
  explicit NetworkTablesReceiver(nt::NetworkTableInstance inst);

  /**
   * Uses a specific NetworkTables instance and a custom root prefix.
   *
   * @param inst NetworkTables instance to publish through.
   * @param prefix Root path (e.g., "/MyRobot").
   */
  NetworkTablesReceiver(nt::NetworkTableInstance inst, std::string_view prefix);

  // LogDataReceiver
  void OnStart() override;
  void OnUpdate(const LogTable& table, int64_t timestamp) override;
  void OnEnd() override;

 private:
  /// Holds the publisher for a key; type depends on the logged value.
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
   * Publishes one key/value pair.
   *
   * @param key Log key (e.g., "/Drivetrain/Speed").
   * @param value Value to publish.
   * @param timestamp Timestamp in microseconds.
   */
  void PublishValue(std::string_view key, const LogValue& value, int64_t timestamp);

  nt::NetworkTableInstance m_inst;
  std::string m_prefix;

  // Per-key publisher cache so we don't recreate publishers every update.
  std::unordered_map<std::string, PublisherVariant> m_publishers;

  // Last published values for simple change detection.
  std::unordered_map<std::string, LogValue> m_lastValues;
};

} 

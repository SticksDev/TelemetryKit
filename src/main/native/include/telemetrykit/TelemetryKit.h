#pragma once

/**
 * TelemetryKit - Main convenience header
 *
 * Include this single header to access all TelemetryKit functionality.
 *
 * Example Usage:
 *
 *   #include <telemetrykit/TelemetryKit.h>
 *
 *   using namespace telemetrykit;
 *
 *   void RobotInit() {
 *     auto& logger = Logger::GetInstance();
 *     logger.Start();
 *     logger.AddReceiver(std::make_unique<WPILogWriter>("/logs"));
 *   }
 *
 *   void RobotPeriodic() {
 *     auto& logger = Logger::GetInstance();
 *     logger.PeriodicBeforeUser();
 *
 *     // Log data
 *     RecordOutput("/Speed", 3.5);
 *     RecordOutput("/Pose", pose);
 *
 *     logger.PeriodicAfterUser();
 *   }
 */

// Core components
#include "telemetrykit/LogValue.h"
#include "telemetrykit/LogTable.h"
#include "telemetrykit/Logger.h"

// IO Interface pattern
#include "telemetrykit/LoggableInputs.h"

// Receivers
#include "telemetrykit/LogDataReceiver.h"
#include "telemetrykit/WPILogWriter.h"
// #include "telemetrykit/NT4Publisher.h"  // TODO: Implement in Phase 5

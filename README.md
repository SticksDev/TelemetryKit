# TelemetryKit

A lightweight, type-safe telemetry library for FRC robots. TelemetryKit provides a unified interface for logging data to NetworkTables, `.wpilog` files, and console output.

**[📚 Documentation](https://sticksdev.github.io/TelemetryKit/)**

## Features

- **Type-safe logging** - Support for primitives, arrays, and WPILib structs
- **Multiple receivers** - Publish to NetworkTables, write to `.wpilog` files, or print to console
- **Change-only optimization** - Only process values when they change

## Installation

### Using Vendor JSON (Recommended)

1. Open VS Code with your FRC project
2. Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)
3. Type "WPILib: Manage Vendor Libraries"
4. Select "Install new libraries (online)"
5. Paste this URL:

```
https://raw.githubusercontent.com/SticksDev/TelemetryKit/refs/heads/2026/TelemetryKit.json
```

## Quick Start

```cpp
#include <telemetrykit/TelemetryKit.h>

auto& logger = tkit::Logger::GetInstance();

// Add receivers
logger.AddReceiver(std::make_unique<tkit::NetworkTablesReceiver>());
logger.AddReceiver(std::make_unique<tkit::WPILogWriter>("/home/lvuser/logs"));

// Start logging
logger.Start();

// Log data every cycle
void RobotPeriodic() {
  tkit::RecordOutput("Drive/Speed", m_drive.GetSpeed());
  tkit::RecordOutput("Vision/Targets", targetCount);
  logger.Periodic();
}
```

## Building from Source

```bash
./gradlew build
```

To install the native toolchain:

```bash
./gradlew installRoboRIOToolchain
```

To publish to local WPILib maven:

```bash
./gradlew copyToWpilibLocal
```

## License

See [LICENSE.txt](LICENSE.txt)

---
icon: lucide/ruler
---

# Units

TelemetryKit supports unit metadata for AdvantageScope's unit-aware graphing and seamless integration with WPILib's units library.

## Overview

Unit metadata allows AdvantageScope to:

- Display values with their proper units (e.g., "3.5 m/s")
- Convert between compatible units when graphing
- Provide better context for logged values

TelemetryKit provides two approaches for unit metadata:

1. **Manual unit strings** - Specify unit abbreviations as strings
2. **WPILib units integration** - Automatic unit extraction from `units::unit_t` types

## Manual Unit Strings

Specify unit metadata as a third parameter when logging:

```cpp
#include <telemetrykit/core/Logger.h>

// Global convenience function
tkit::RecordOutput("/Speed", 3.5, "m/s");
tkit::RecordOutput("/Angle", 1.57, "radians");
tkit::RecordOutput("/Current", 42.0, "amps");

// Or via Logger instance
auto& logger = tkit::Logger::GetInstance();
logger.RecordOutput("/Distance", 5.0, "meters");
```

### LogTable with Units

Units work with LogTable as well:

```cpp
tkit::LogTable table;
table.Put("/Speed", 3.5, "m/s");
table.Put("/Angle", 1.57, "radians");

// Retrieve unit metadata
std::string unit = table.GetUnit("/Speed");  // Returns "m/s"

// Get all units in the table
auto allUnits = table.GetAllUnits();
// Returns: {"/Speed" -> "m/s", "/Angle" -> "radians"}
```

### Subtables with Units

Units are preserved through subtables:

```cpp
tkit::LogTable rootTable;
auto gyroTable = rootTable.GetSubtable("/Gyro");

gyroTable.Put("Yaw", 45.0, "degrees");
gyroTable.Put("Pitch", 10.0, "degrees");

// Units accessible from root table
rootTable.GetUnit("/Gyro/Yaw");  // Returns "degrees"
```

## WPILib Units Integration

For automatic unit extraction from WPILib unit types, include the Units header:

```cpp
#include <telemetrykit/core/Units.h>

// WPILib units headers
#include <units/length.h>
#include <units/velocity.h>
#include <units/angle.h>
```

### RecordOutput with Unit Types

The `tkit::RecordOutput` function is overloaded for WPILib unit types:

```cpp
#include <telemetrykit/core/Units.h>
#include <units/length.h>
#include <units/velocity.h>
#include <units/angle.h>
#include <units/voltage.h>

// Automatic unit extraction
tkit::RecordOutput("/Distance", units::meter_t{3.5});
// Logs 3.5 with unit "m"

tkit::RecordOutput("/Angle", units::degree_t{90.0});
// Logs 90.0 with unit "deg"

tkit::RecordOutput("/Speed", units::meters_per_second_t{2.5});
// Logs 2.5 with unit "mps"

tkit::RecordOutput("/Voltage", units::volt_t{12.6});
// Logs 12.6 with unit "V"
```

### Put with Unit Types

Use `tkit::Put` helper for LogTable operations:

```cpp
#include <telemetrykit/core/Units.h>

tkit::LogTable table;

// Automatic unit extraction
tkit::Put(table, "/Distance", units::meter_t{5.0});
tkit::Put(table, "/Angle", units::degree_t{45.0});
tkit::Put(table, "/Speed", units::meters_per_second_t{3.5});

// Verify units were set
table.GetUnit("/Distance");  // Returns "m"
table.GetUnit("/Angle");     // Returns "deg"
table.GetUnit("/Speed");     // Returns "mps"
```

### Subtables with Unit Types

```cpp
tkit::LogTable rootTable;
auto driveTable = rootTable.GetSubtable("/Drive");

tkit::Put(driveTable, "LeftVelocity", units::meters_per_second_t{2.0});
tkit::Put(driveTable, "RightVelocity", units::meters_per_second_t{2.1});

// Units accessible from root table
rootTable.GetUnit("/Drive/LeftVelocity");  // Returns "mps"
```

## Supported Unit Types

TelemetryKit works with any WPILib unit type. See the [WPILib Units Documentation](https://docs.wpilib.org/en/stable/docs/software/basic-programming/cpp-units.html) for the full list of available types.

!!! note "WPILib Abbreviations"
    Unit abbreviations come from WPILib's `units::abbreviation()` function. Some may differ from common notation (e.g., "mps" instead of "m/s", "rad_per_s" instead of "rad/s").

## How It Works

TelemetryKit uses C++ type traits to detect WPILib unit types at compile time via SFINAE. When a unit type is detected:

1. `value.value()` extracts the numeric value
2. `units::abbreviation(UnitType{})` gets the unit string
3. Both are passed to the standard logging functions

```cpp
template<typename T>
inline auto RecordOutput(std::string_view key, const T& value)
    -> std::enable_if_t<detail::is_unit_type_v<T>, void>
{
    std::string unit = detail::GetUnitAbbreviation<T>();
    Logger::GetInstance().RecordOutput(key, value.value(), unit);
}
```

## Real-World Example

```cpp
#include <telemetrykit/core/Units.h>
#include <telemetrykit/core/Logger.h>
#include <units/length.h>
#include <units/velocity.h>
#include <units/angle.h>
#include <units/voltage.h>
#include <units/current.h>

class DriveSubsystem {
public:
  void Periodic() {
    // Get sensor readings as unit types
    auto leftVelocity = m_leftEncoder.GetVelocity();   // units::meters_per_second_t
    auto rightVelocity = m_rightEncoder.GetVelocity(); // units::meters_per_second_t
    auto gyroAngle = m_gyro.GetYaw();                  // units::degree_t

    // Log with automatic unit extraction
    tkit::RecordOutput("/Drive/LeftVelocity", leftVelocity);
    tkit::RecordOutput("/Drive/RightVelocity", rightVelocity);
    tkit::RecordOutput("/Drive/GyroAngle", gyroAngle);

    // Motor diagnostics
    tkit::RecordOutput("/Drive/LeftMotor/Voltage", m_leftMotor.GetVoltage());
    tkit::RecordOutput("/Drive/LeftMotor/Current", m_leftMotor.GetCurrent());
  }
};
```

In AdvantageScope, these values will display with their proper units and support unit-aware graphing features.

## Unit Metadata in WPILOG

Unit metadata is stored in the WPILOG file's entry metadata as JSON:

```json
{"unit":"m"}
```

This allows AdvantageScope to read and use the unit information when loading log files.

## Next Steps

- Learn about [LogValue](logvalue.md) for all supported types
- See [Struct Logging](../advanced/structs.md) for WPILib geometry types
- Check out [Examples](../examples/basic.md) for practical usage patterns

---

[LogValue →](logvalue.md){ .md-button .md-button--primary }

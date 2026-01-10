---
icon: lucide/box
---

# LogValue

Type-safe container that can hold any loggable value using `std::variant`.

## Overview

Provides:

- Compile-time safety - Template methods catch type errors at compile time
- Runtime flexibility - Can store any supported type in a single container
- Zero-copy where possible - Efficient storage and retrieval
- Automatic type detection - Templates deduce types from arguments

## Supported Types

### Primitives

=== "Boolean"

    ```cpp
    tkit::LogValue boolVal(true);
    tkit::Logger::RecordOutput("System/Enabled", true);
    ```

=== "Integer"

    ```cpp
    tkit::LogValue intVal(42);  // (1)!
    tkit::Logger::RecordOutput("Encoder/Ticks", 1234);

    // Most integer types work
    int count = 10;
    long bigNum = 1000000L;
    tkit::Logger::RecordOutput("Counter", count);  // (2)!
    tkit::Logger::RecordOutput("BigValue", bigNum);
    ```

    1. Accepts `int`, `long`, `int64_t`, and most other integer primitives
    2. All integers are stored internally as `int64_t`

=== "Float"

    ```cpp
    tkit::LogValue floatVal(3.14f);
    tkit::Logger::RecordOutput("Motor/Voltage", 12.5f);
    ```

=== "Double"

    ```cpp
    tkit::LogValue doubleVal(2.718);
    tkit::Logger::RecordOutput("Odometry/X", 1.5);
    ```

=== "String"

    ```cpp
    tkit::LogValue stringVal("Autonomous Mode");
    tkit::Logger::RecordOutput("Auto/Selected", std::string("3 Ball Auto"));

    // String views work too
    std::string_view mode = "Test";
    tkit::Logger::RecordOutput("Robot/Mode", std::string(mode));
    ```

### Arrays

All array types use `std::vector` and support dynamic sizing.

=== "Boolean Array"

    ```cpp
    std::vector<bool> flags = {true, false, true, true};
    tkit::LogValue arrayVal(flags);
    tkit::Logger::RecordOutput("Subsystems/Enabled", flags);
    ```

=== "Integer Array"

    ```cpp
    std::vector<int64_t> encoderValues = {100, 200, 300, 400};
    tkit::Logger::RecordOutput("Swerve/EncoderTicks", encoderValues);
    ```

=== "Float Array"

    ```cpp
    std::vector<float> voltages = {12.1f, 11.9f, 12.0f, 12.2f};
    tkit::Logger::RecordOutput("Motors/Voltages", voltages);
    ```

=== "Double Array"

    ```cpp
    std::vector<double> wheelSpeeds = {1.5, 1.6, 1.4, 1.5};
    tkit::Logger::RecordOutput("Drive/WheelSpeeds", wheelSpeeds);
    ```

=== "String Array"

    ```cpp
    std::vector<std::string> moduleNames = {"FL", "FR", "BL", "BR"};
    tkit::Logger::RecordOutput("Swerve/ModuleNames", moduleNames);
    ```

### Structs

WPILib structs are fully supported with automatic schema publishing.

=== "Single Struct"

    ```cpp
    #include <frc/geometry/Pose2d.h>
    #include <telemetrykit/core/StructLogger.h>

    frc::Pose2d pose{1.5_m, 2.3_m, frc::Rotation2d{45_deg}};

    // Use MakeStructValue helper
    tkit::LogValue structVal = tkit::MakeStructValue(pose);
    tkit::Logger::RecordOutput("Robot/Pose", tkit::MakeStructValue(pose));
    ```

=== "Struct Array"

    ```cpp
    #include <frc/kinematics/SwerveModuleState.h>
    #include <telemetrykit/core/StructLogger.h>

    std::vector<frc::SwerveModuleState> states = {
      {1.5_mps, frc::Rotation2d{0_deg}},
      {1.6_mps, frc::Rotation2d{90_deg}},
      {1.4_mps, frc::Rotation2d{180_deg}},
      {1.5_mps, frc::Rotation2d{270_deg}}
    };

    // Use MakeStructArrayValue helper
    tkit::Logger::RecordOutput("Swerve/States",
      tkit::MakeStructArrayValue(std::span(states)));
    ```

See [Struct Logging](../advanced/structs.md) for comprehensive struct support details.

### Raw Binary Data

For custom binary data or unsupported types:

```cpp
std::vector<uint8_t> rawData = {0x01, 0x02, 0x03, 0x04};
tkit::LogValue rawVal(rawData, false);  // false = not a struct
tkit::Logger::RecordOutput("Custom/Data", rawVal);
```

## Type System

### LogType Enum

```cpp
enum class LogType {
  kBoolean,        // Single bool
  kInt64,          // Single int64_t
  kFloat,          // Single float
  kDouble,         // Single double
  kString,         // Single std::string

  kBooleanArray,   // std::vector<bool>
  kInt64Array,     // std::vector<int64_t>
  kFloatArray,     // std::vector<float>
  kDoubleArray,    // std::vector<double>
  kStringArray,    // std::vector<std::string>

  kRaw,            // std::vector<uint8_t> (binary data)
  kStruct,         // Serialized WPILib struct
  kStructArray     // Array of serialized structs
};
```

### Runtime Type Information

Every LogValue knows its type at runtime:

```cpp
tkit::LogValue value(42);

// Check the type
tkit::LogType type = value.GetType();  // Returns LogType::kInt64

// Type checking
if (value.Is<int64_t>()) {
  // Safe to retrieve as int64_t
}

// Get type as string
std::string typeStr = value.GetTypeString();  // Returns "int64"

// For structs, returns the struct type name
frc::Pose2d pose{1_m, 2_m, 0_deg};
auto structVal = tkit::MakeStructValue(pose);
std::string structType = structVal.GetTypeString();  // Returns "Pose2d"
```

## Retrieving Values

### Type-Safe Retrieval

Use the `Get<T>()` template method to retrieve values:

```cpp
tkit::LogValue value(3.14);

// Retrieve the value (type must match exactly)
double d = value.Get<double>();

// Check type before retrieving to avoid exceptions
if (value.Is<double>()) {
  double d = value.Get<double>();
  std::cout << "Value: " << d << std::endl;
}
```

!!! danger "Type Mismatch Throws Exception"
    Calling `Get<T>()` with the wrong type throws `std::bad_variant_access`. Always check the type first with `Is<T>()` or `GetType()`.

### Common Retrieval Patterns

```cpp
// Pattern 1: Check type enum
if (value.GetType() == tkit::LogType::kDouble) {
  double d = value.Get<double>();
}

// Pattern 2: Check with template
if (value.Is<std::vector<double>>()) {
  const auto& array = value.Get<std::vector<double>>();
  for (double val : array) {
    // Process array values
  }
}

// Pattern 3: Switch on type
switch (value.GetType()) {
  case tkit::LogType::kDouble:
    ProcessDouble(value.Get<double>());
    break;
  case tkit::LogType::kDoubleArray:
    ProcessDoubleArray(value.Get<std::vector<double>>());
    break;
  // ... other cases
}
```

## Value Comparison

LogValue supports equality comparison:

```cpp
tkit::LogValue a(3.14);
tkit::LogValue b(3.14);
tkit::LogValue c(2.71);

bool same = (a == b);      // true
bool different = (a == c); // false

// Works with all types
tkit::LogValue arr1(std::vector<double>{1.0, 2.0});
tkit::LogValue arr2(std::vector<double>{1.0, 2.0});
bool arrSame = (arr1 == arr2);  // true
```

Used internally for field-change-only optimization - receivers only log values when they change.

## Custom Type Mapping

### Type String for Structs

When creating struct LogValues, provide the WPILib type name:

```cpp
// MakeStructValue automatically extracts type name
auto pose = frc::Pose2d{1_m, 2_m, 0_deg};
auto value = tkit::MakeStructValue(pose);  // Type string = "Pose2d"

// Manual construction (not recommended)
std::vector<uint8_t> serialized = /* ... */;
tkit::LogValue value(serialized, "Pose2d");
```

### Schema Storage

Struct LogValues store their schema for automatic publishing:

```cpp
auto pose = frc::Pose2d{1_m, 2_m, 0_deg};
auto value = tkit::MakeStructValue(pose);

// Schema automatically included
bool hasSchema = value.HasSchema();  // true
const auto& schema = value.GetSchema();

// Nested schemas (e.g., Pose2d includes Translation2d, Rotation2d)
const auto& allSchemas = value.GetAllSchemas();
// Returns: {"Pose2d" -> schema, "Translation2d" -> schema, "Rotation2d" -> schema}
```

See [Struct Logging](../advanced/structs.md) for details on schema handling.

## Performance Considerations

### Move Semantics

LogValue supports move semantics for efficient transfers:

```cpp
std::vector<double> largeArray(10000, 3.14);

// Efficient: moves the vector into LogValue (no copy)
tkit::LogValue value(std::move(largeArray));
// largeArray is now empty

// Also works with RecordOutput
std::vector<double> data(10000);
tkit::Logger::RecordOutput("Data", std::move(data));
```

### Small Value Optimization

Primitive types (bool, int64_t, float, double) are stored directly in the variant without heap allocation:

```cpp
// No heap allocation
tkit::LogValue intVal(42);
tkit::LogValue floatVal(3.14f);
tkit::LogValue boolVal(true);

// Single heap allocation for vector
tkit::LogValue arrayVal(std::vector<double>{1.0, 2.0, 3.0});
```

### Copy Avoidance

Use references to avoid copying when retrieving arrays:

```cpp
tkit::LogValue value(std::vector<double>{1.0, 2.0, 3.0});

// Bad: copies the entire vector
auto copy = value.Get<std::vector<double>>();

// Good: const reference, no copy
const auto& ref = value.Get<std::vector<double>>();
```

## Type Conversion Table

| C++ Type | LogType | Storage | Notes |
|----------|---------|---------|-------|
| `bool` | `kBoolean` | Direct | No heap allocation |
| `int`, `long`, `int64_t` | `kInt64` | Direct | All integers stored as int64_t |
| `float` | `kFloat` | Direct | No heap allocation |
| `double` | `kDouble` | Direct | No heap allocation |
| `std::string` | `kString` | Heap | String contents on heap |
| `std::vector<bool>` | `kBooleanArray` | Heap | |
| `std::vector<int64_t>` | `kInt64Array` | Heap | |
| `std::vector<float>` | `kFloatArray` | Heap | |
| `std::vector<double>` | `kDoubleArray` | Heap | |
| `std::vector<std::string>` | `kStringArray` | Heap | |
| `frc::Pose2d` | `kStruct` | Heap | Via `MakeStructValue()` |
| `frc::SwerveModuleState` | `kStruct` | Heap | Via `MakeStructValue()` |
| `std::vector<frc::Pose2d>` | `kStructArray` | Heap | Via `MakeStructArrayValue()` |

!!! warning "Incomplete Table"
    This table is not complete. Most C++ primitive types (int, short, unsigned, etc.) are automatically converted to the listed types. Any WPILib struct that implements `wpi::Struct<T>` can be logged using `MakeStructValue()` or `MakeStructArrayValue()` for 
    lists.

## Next Steps

- Learn about [Receivers](../receivers/overview.md) that consume LogValues
- See [Struct Logging](../advanced/structs.md) for working with WPILib geometry types
- Check out [Examples](../examples/basic.md) for practical usage patterns

---

[Receivers →](../receivers/overview.md){ .md-button .md-button--primary }

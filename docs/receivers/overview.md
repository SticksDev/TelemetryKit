---
icon: lucide/radio
---

# Receivers

Receivers process logged data from the `LogTable` during each `Logger::Periodic()` call. They implement the `LogDataReceiver` interface with three lifecycle methods: `OnStart()`, `OnUpdate()`, and `OnEnd()`.

TelemetryKit includes three built-in receivers for different output targets.

## Built-in Receivers

### NetworkTablesReceiver

Publishes data to NetworkTables 4 for real-time dashboard viewing.

```cpp
#include <telemetrykit/receiver/NetworkTablesReceiver.h>

logger.AddReceiver(std::make_unique<tkit::NetworkTablesReceiver>());
```

**What it does:**

- Publishes all LogValue types to NT4 topics
- Publishes struct schemas
- Only sends changed values

**Configuration:**

```cpp
// Use a custom NT instance (optional)
auto inst = nt::NetworkTableInstance::Create();
auto receiver = std::make_unique<tkit::NetworkTablesReceiver>(inst);
```

Works with Shuffleboard, Glass, AdvantageScope, and Elastic.

### WPILogWriter

Writes data to `.wpilog` files for post-match analysis.

```cpp
#include <telemetrykit/receiver/WPILogWriter.h>

logger.AddReceiver(std::make_unique<tkit::WPILogWriter>("/home/lvuser/logs"));
```

**What it does:**

- Writes to timestamped `.wpilog` files
- Serializes structs with proper type strings
- Only writes changed values

Files are saved as `TelemetryKit_YYYYMMDD_HHMMSS.wpilog` and can be viewed in [AdvantageScope](https://github.com/Mechanical-Advantage/AdvantageScope).

### ConsoleReceiver

Prints data to console for debugging.

```cpp
#include <telemetrykit/receiver/ConsoleReceiver.h>

logger.AddReceiver(std::make_unique<tkit::ConsoleReceiver>());
```

**What it does:**

- Prints human-readable output
- Only prints changed values by default

**Configuration:**

```cpp
// Custom prefix
auto receiver = std::make_unique<tkit::ConsoleReceiver>("[MyRobot]");

// Print all values every cycle
receiver->SetPrintOnlyChanges(false);
```

!!! warning
    For debugging only. Disable on competition robots.

## Using Multiple Receivers

You can use multiple receivers simultaneously:

```cpp
auto& logger = tkit::Logger::GetInstance();

// Real-time dashboard viewing
logger.AddReceiver(std::make_unique<tkit::NetworkTablesReceiver>());

// Post-match analysis
logger.AddReceiver(std::make_unique<tkit::WPILogWriter>("/home/lvuser/logs"));

// Debug output (disable on competition robot)
#ifdef DEBUG
logger.AddReceiver(std::make_unique<tkit::ConsoleReceiver>());
#endif

logger.Start();
```

!!! tip
    Receivers operate independently. If one fails, others continue.

## Receiver Lifecycle

All receivers implement three lifecycle methods:

``` mermaid
graph LR
  Start([Start]) -->|Logger::Start| OnStart[OnStart]
  OnStart -->|Logger::Periodic| OnUpdate[OnUpdate]
  OnUpdate -->|Logger::Periodic| OnUpdate
  OnUpdate -->|Logger::End| OnEnd[OnEnd]
  OnEnd --> End([End])
```

**`OnStart()`** - Called once when `Logger::Start()` is invoked. Use for initialization (open files, create publishers, allocate resources).

**`OnUpdate(const LogTable& table, int64_t timestamp)`** - Called every `Logger::Periodic()` call. Process logged data from the table.

**`OnEnd()`** - Called when `Logger::End()` is invoked. Use for cleanup (close files, flush buffers, release resources).

## Custom Receivers

Implement the `LogDataReceiver` interface:

```cpp
#include <telemetrykit/receiver/LogDataReceiver.h>

class CustomReceiver : public tkit::LogDataReceiver {
 public:
  void OnStart() override {
    // Initialize resources
    std::cout << "Custom receiver started!" << std::endl;  // (1)!
  }

  void OnUpdate(const tkit::LogTable& table, int64_t timestamp) override {
    // Process data
    auto entries = table.GetAllEntries();
    for (const auto& [key, value] : entries) {
      // Do something with key and value
      ProcessEntry(key, value, timestamp);  // (2)!
    }
  }

  void OnEnd() override {
    // Cleanup
    std::cout << "Custom receiver stopped!" << std::endl;  // (3)!
  }

 private:
  void ProcessEntry(const std::string& key, const tkit::LogValue& value, int64_t timestamp) {
    // Your custom logic here
  }
};
```

1. Open files, connect to databases, or initialize network connections
2. Process each entry - use `value.GetType()` and `value.Get<T>()` to extract data
3. Close connections and flush buffers before returning

## Performance Considerations

### Change-Only Optimization

`NetworkTablesReceiver` and `WPILogWriter` only process changed values:

```cpp
// Internal tracking
std::unordered_map<std::string, LogValue> m_lastValues;  // (1)!

// Only publish/write if value changed
if (m_lastValues[key] != value) {  // (2)!
  PublishValue(key, value, timestamp);
  m_lastValues[key] = value;  // (3)!
}
```

1. Each receiver maintains a cache of last-seen values
2. Compares current value against cached value using `LogValue::operator==`
3. Updates cache only when value changes

---

[Next: Struct Logging →](../advanced/structs.md){ .md-button .md-button--primary }

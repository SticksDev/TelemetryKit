#pragma once

#include <concepts>
#include <string>
#include <string_view>

#include <units/base.h>

#include "telemetrykit/core/Logger.h"

namespace tkit {

/**
 * Units.h
 *
 * Integration helpers for logging WPILib units with TelemetryKit.
 * Extracts numeric values and unit abbreviations automatically.
 *
 * Example:
 *   tkit::RecordOutput("/Distance", units::meter_t{3.5});
 *   tkit::RecordOutput("/Angle", units::degree_t{45.0});
 *   tkit::RecordOutput("/Speed", units::meters_per_second_t{2.5});
 */

// Concept for WPILib unit types
template<typename T>
concept UnitType = requires(T t) {
    { t.value() } -> std::convertible_to<double>;
    typename T::unit_type;
};

namespace detail {

// Returns the abbreviation string for a unit type
template<UnitType T>
std::string GetUnitAbbreviation() {
    return std::string(units::abbreviation(T{}));
}

}  // namespace detail

/**
 * Records a value from a WPILib unit type.
 *
 * The numeric value and unit abbreviation are logged automatically.
 *
 * Example:
 *   tkit::RecordOutput("/Distance", units::meter_t{3.5}); // 3.5, "m"
 *   tkit::RecordOutput("/Angle", units::degree_t{90.0});  // 90.0, "deg"
 */
template<UnitType T>
inline void RecordOutput(std::string_view key, const T& value) {
    std::string unit = detail::GetUnitAbbreviation<T>();
    Logger::GetInstance().RecordOutput(key, value.value(), unit);
}

/**
 * LogTable::Put overload for WPILib unit types.
 */
template<UnitType T>
inline void Put(LogTable& table, std::string_view key, const T& value) {
    std::string unit = detail::GetUnitAbbreviation<T>();
    table.Put(key, value.value(), unit);
}

}  // namespace tkit

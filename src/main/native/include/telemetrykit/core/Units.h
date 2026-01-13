#pragma once

#include <string>
#include <string_view>
#include <type_traits>

#include <units/base.h>

#include "telemetrykit/core/Logger.h"

namespace tkit {

/**
 * Units.h - WPILib units library integration for TelemetryKit
 *
 * Provides automatic unit metadata extraction when logging WPILib unit types.
 *
 * Usage:
 *   #include <telemetrykit/core/Units.h>
 *
 *   tkit::RecordOutput("/Distance", units::meter_t{3.5});
 *   tkit::RecordOutput("/Angle", units::degree_t{45.0});
 *   tkit::RecordOutput("/Speed", units::meters_per_second_t{2.5});
 */

namespace detail {

// Type trait to detect if T is a units::unit_t type
template<typename T, typename = void>
struct is_unit_type : std::false_type {};

template<typename T>
struct is_unit_type<T, std::void_t<
    decltype(std::declval<T>().value()),
    typename T::unit_type
>> : std::true_type {};

template<typename T>
inline constexpr bool is_unit_type_v = is_unit_type<T>::value;

// Get the unit abbreviation string for a units::unit_t type
template<typename UnitType>
std::string GetUnitAbbreviation() {
    return std::string(units::abbreviation(UnitType{}));
}

}  // namespace detail

/**
 * Record an output value from a WPILib unit type.
 *
 * Automatically extracts the numeric value and unit abbreviation.
 *
 * Examples:
 *   tkit::RecordOutput("/Distance", units::meter_t{3.5});
 *   // Logs 3.5 with unit "m"
 *
 *   tkit::RecordOutput("/Angle", units::degree_t{90.0});
 *   // Logs 90.0 with unit "deg"
 *
 *   tkit::RecordOutput("/Speed", units::feet_per_second_t{10.0});
 *   // Logs 10.0 with unit "ft/s"
 */
template<typename T>
inline auto RecordOutput(std::string_view key, const T& value)
    -> std::enable_if_t<detail::is_unit_type_v<T>, void>
{
    std::string unit = detail::GetUnitAbbreviation<T>();
    Logger::GetInstance().RecordOutput(key, value.value(), unit);
}

/**
 * LogTable::Put overload for WPILib unit types.
 */
template<typename T>
inline auto Put(LogTable& table, std::string_view key, const T& value)
    -> std::enable_if_t<detail::is_unit_type_v<T>, void>
{
    std::string unit = detail::GetUnitAbbreviation<T>();
    table.Put(key, value.value(), unit);
}

}  // namespace tkit

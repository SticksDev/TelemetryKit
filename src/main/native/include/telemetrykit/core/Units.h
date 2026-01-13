#pragma once

#include <string>
#include <string_view>
#include <type_traits>

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

namespace detail {

// Detects whether a type is a WPILib units::unit_t
template<typename T, typename = void>
struct is_unit_type : std::false_type {};

template<typename T>
struct is_unit_type<T, std::void_t<
    decltype(std::declval<T>().value()),
    typename T::unit_type
>> : std::true_type {};

template<typename T>
inline constexpr bool is_unit_type_v = is_unit_type<T>::value;

// Returns the abbreviation string for a unit type
template<typename UnitType>
std::string GetUnitAbbreviation() {
    return std::string(units::abbreviation(UnitType{}));
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

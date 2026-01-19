#pragma once

#include <concepts>
#include <string>
#include <string_view>

#include <units/base.h>

#include "telemetrykit/core/Logger.h"

namespace tkit {

/**
 * Helpers for logging WPILib unit types.
 *
 * Extracts the numeric value and unit abbreviation automatically.
 */

// Concept for WPILib unit types
template<typename T>
concept UnitType = requires(T t) {
  { t.value() } -> std::convertible_to<double>;
  typename T::unit_type;
};

namespace detail {

// Returns the abbreviation for a unit type (e.g. "m", "deg").
template<UnitType T>
std::string GetUnitAbbreviation() {
  return std::string(units::abbreviation(T{}));
}

}  // namespace detail

/**
 * Logs a WPILib unit value through the global Logger.
 */
template<UnitType T>
inline void RecordOutput(std::string_view key, const T& value) {
  Logger::GetInstance().RecordOutput(
      key,
      value.value(),
      detail::GetUnitAbbreviation<T>());
}

/**
 * LogTable::Put overload for WPILib unit values.
 */
template<UnitType T>
inline void Put(LogTable& table, std::string_view key, const T& value) {
  table.Put(
      key,
      value.value(),
      detail::GetUnitAbbreviation<T>());
}

} 
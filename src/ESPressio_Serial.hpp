#pragma once

#include "ESPressio_SerialTypes.hpp"

/*
 * Core ESPressio Serial deliberately has no ESPressio Event or Serializable
 * dependency.
 *
 * Event Transport monitoring is opt-in:
 *
 *     #include <ESPressio_EventMonitor.hpp>
 *
 * or:
 *
 *     #include <ESPressio_SerialEventMonitoring.hpp>
 */

#pragma once

#include "ESPressio_SerialTypes.hpp"
#include "ESPressio_DiagnosticTypes.hpp"

/*
 * Optional facilities are selected explicitly. Core Serial remains free of
 * mandatory ESPressio-library dependencies.
 *
 * Logging / diagnostics:
 *   ESPressio_Logging.hpp
 *   ESPressio_SystemClockMonitor.hpp
 *   ESPressio_ThreadMonitor.hpp
 *   ESPressio_EventMonitor.hpp
 *   ESPressio_DiagnosticMonitor.hpp
 *
 * Observable subsystem monitors:
 *   ESPressio_CommandMonitor.hpp          -> Command >= 0.3.0 < 1.0.0
 *   ESPressio_SecurityMonitor.hpp         -> Security >= 0.2.0 < 1.0.0
 *   ESPressio_SocketWorkerMonitor.hpp     -> Sockets >= 0.5.0 < 1.0.0
 *   ESPressio_SocketSecuritySessionMonitor.hpp
 *   ESPressio_ESPNowTransportMonitor.hpp  -> ESP-Now >= 0.5.0 < 1.0.0
 *
 * Interactive integrations:
 *   ESPressio_CommandConsole.hpp
 *   ESPressio_EventConsole.hpp
 */

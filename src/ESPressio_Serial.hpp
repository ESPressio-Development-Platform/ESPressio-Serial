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
 *   ESPressio_CommandMonitor.hpp          -> Command >= 1.0.1 < 2.0.0
 *   ESPressio_SecurityMonitor.hpp         -> Security >= 0.4.0 < 1.0.0
 *   ESPressio_SocketWorkerMonitor.hpp     -> Sockets >= 0.7.1 < 1.0.0
 *   ESPressio_SocketSecuritySessionMonitor.hpp
 *   ESPressio_ESPNowTransportMonitor.hpp  -> ESP-Now >= 0.8.1 < 1.0.0
 *   ESPressio_WiFiMonitor.hpp             -> WiFi >= 0.1.0 < 1.0.0
 *   ESPressio_StateMonitor.hpp            -> State, optional and header-selected
 *
 * Interactive integrations:
 *   ESPressio_CommandConsole.hpp
 *   ESPressio_EventConsole.hpp
 */

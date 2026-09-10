#pragma once

#include "ESPressio_SerialTypes.hpp"

/*
 * Optional facilities are selected explicitly.
 *
 * Logging Sink:
 *   ESPressio_SerialLogging.hpp            -> ESPressio-Logging + SerialLogSink
 *
 * Logging / diagnostic monitors:
 *   ESPressio_SystemClockMonitor.hpp
 *   ESPressio_ThreadMonitor.hpp
 *   ESPressio_EventMonitor.hpp
 *   ESPressio_DiagnosticMonitor.hpp
 *
 * Observable subsystem monitors:
 *   ESPressio_CommandMonitor.hpp          -> Command
 *   ESPressio_SecurityMonitor.hpp         -> Security
 *   ESPressio_SocketWorkerMonitor.hpp     -> Sockets
 *   ESPressio_SocketSecuritySessionMonitor.hpp
 *   ESPressio_ESPNowTransportMonitor.hpp  -> ESP-Now
 *   ESPressio_WiFiMonitor.hpp             -> WiFi
 *   ESPressio_StateMonitor.hpp            -> State, optional and header-selected
 *
 * Interactive integrations:
 *   ESPressio_CommandConsole.hpp
 *   ESPressio_EventConsole.hpp
 */

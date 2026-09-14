#pragma once

/*
 * Optional facilities are selected explicitly.
 *
 * Logging sink:
 *   ESPressio_SerialLogging.hpp                 -> ESPressio-Logging
 *
 * Final diagnostics:
 *   ESPressio_SystemClockMonitor.hpp            -> Timing observer
 *   ESPressio_ThreadMonitor.hpp                 -> caller-owned IThread snapshot
 *   ESPressio_EventMonitor.hpp                  -> Event descriptors
 *   ESPressio_CommandMonitor.hpp                -> Command descriptors
 *   ESPressio_StateMonitor.hpp                  -> State descriptors/read-only dynamic read
 *   ESPressio_SecurityMonitor.hpp               -> Security
 *   ESPressio_SocketAdapterTransportMonitor.hpp -> Sockets final A2 transport snapshot
 *   ESPressio_SocketSecuritySessionMonitor.hpp  -> Sockets/Security session observer
 *   ESPressio_ESPNowRadioMonitor.hpp            -> ESP-NOW final Radio provider snapshot
 *   ESPressio_WiFiMonitor.hpp                   -> WiFi runtime observer/state
 *   ESPressio_DiagnosticMonitor.hpp
 *
 * Interactive integrations:
 *   ESPressio_CommandConsole.hpp
 *   ESPressio_EventConsole.hpp
 *
 * Serial intentionally provides no ThreadManager, ESPNowTransport, SocketWorker,
 * WiFi EventThread, Primitive transport, retry, scheduler, registry compatibility layer,
 * or predecessor Event-transport monitoring configuration surface.
 */

# Subsystem Monitors

Serial provides opt-in diagnostic monitors for other ESPressio subsystems.

The core rule is simple: **observe the source library's native public Observer/API surface; do not invent a parallel lifecycle model.**

Current integrations include monitors for areas such as:

- Timing/System Clock;
- Threads;
- Event/Event Transport;
- Command;
- Security;
- Sockets;
- ESP-NOW;
- WiFi.

## WiFi example

`WiFiMonitor` consumes ESPressio-WiFi public observer types rather than polling Arduino WiFi directly. AP and Client contexts remain distinct and platform-native WiFi types do not cross the integration boundary.

## Event diagnostics

Structured Event/ESPB diagnostics use bounded traversal. Malformed or outside-limit payloads fall back to bounded hexadecimal diagnostics rather than allowing diagnostic parsing itself to become an unbounded/failing workload.

## Monitor lifetime

Keep observer/registration handles alive according to the originating library's lifecycle contract. Serial should not weaken source-library ownership semantics merely to make a monitor easier to instantiate.

## Adding a monitor

A new monitor should:

1. depend only on the source library's public API;
2. remain optional so Serial core does not acquire a mandatory domain dependency;
3. avoid credentials/secrets and other sensitive state;
4. keep formatting and buffering bounded;
5. avoid blocking or expensive work in source-library callbacks where possible;
6. preserve the source library's terminology and lifecycle semantics.

If a source library lacks an appropriate public observation surface, improve that library's abstraction rather than reaching through Serial into its private/platform implementation.
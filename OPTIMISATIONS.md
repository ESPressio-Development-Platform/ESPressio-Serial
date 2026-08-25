# ESPressio Serial optimisation and lifecycle log

This file records the current mutable-release optimisation/lifecycle validation round chronologically. Version numbers remain unchanged until the coordinated branch set is finalized.

## 2026-08-25 — Issue #39 — Explicit WiFi Event monitor lifecycle

- Migrated `WiFiEventMonitor` from the ambiguous `EventThread(false)` constructor to `EventThread(ThreadReleasePolicy::ExplicitRelease)`.
- Explicitly disabled `StartOnInitialize` for the monitor. Construction and initialization may register/prepare the EventThread, but application lifecycle code must call `Start()` before its loop can execute.
- This aligns Serial with the active Threads/Event lifecycle branches and prevents a diagnostic singleton/worker from becoming runnable while application `setup()` is still assembling WiFi, ESP-NOW, Event transport, listeners, and command infrastructure.
- No released version number was changed.

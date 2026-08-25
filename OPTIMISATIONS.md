# ESPressio Serial optimisation and lifecycle log

This file records the current mutable-release optimisation/lifecycle validation round chronologically. Version numbers remain unchanged until the coordinated branch set is finalized.

## 2026-08-25 — Issue #39 — Explicit WiFi Event monitor lifecycle

- Migrated `WiFiEventMonitor` from the ambiguous `EventThread(false)` constructor to `EventThread(ThreadReleasePolicy::ExplicitRelease)`.
- Explicitly disabled `StartOnInitialize` for the monitor. Construction and initialization may prepare the EventThread, but application lifecycle code must call `Start()` before its loop can execute.
- Removed Event-listener registration from the constructor. Merely constructing a global `WiFiEventMonitor` no longer touches `EventManager` during C++ static initialization; listeners are registered only when `Initialize()` is called explicitly.
- Updated the WiFi Event monitor integration workflow to compile against the active Serializable, Threads, Event and WiFi working branches and to model `ThreadManager::Initialize()` followed by explicit monitor `Start()`.
- This aligns Serial with the active lifecycle branches and prevents a diagnostic worker from becoming runnable or registering Event infrastructure while application `setup()` is still assembling WiFi, ESP-NOW, Event transport, listeners and command infrastructure.
- No released version number was changed.

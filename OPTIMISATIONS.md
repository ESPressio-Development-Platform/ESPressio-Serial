# ESPressio Serial optimisation and lifecycle log

This file records the current mutable-release optimisation/lifecycle validation round chronologically. Version numbers remain unchanged until the coordinated branch set is finalized.

## 2026-08-25 — Issue #39 — Explicit WiFi Event monitor lifecycle

- Migrated `WiFiEventMonitor` from the ambiguous `EventThread(false)` constructor to `EventThread(ThreadReleasePolicy::ExplicitRelease)`.
- Explicitly disabled `StartOnInitialize` for the monitor. Construction and initialization may prepare the EventThread, but application lifecycle code must call `Start()` before its loop can execute.
- Removed Event-listener registration from the constructor. Merely constructing a global `WiFiEventMonitor` no longer touches `EventManager` during C++ static initialization; listeners are registered only when `Initialize()` is called explicitly.
- Updated the WiFi Event monitor integration workflow to compile against the active Serializable, Threads, Event and WiFi working branches and to model `ThreadManager::Initialize()` followed by explicit monitor `Start()`.
- This aligns Serial with the active lifecycle branches and prevents a diagnostic worker from becoming runnable or registering Event infrastructure while application `setup()` is still assembling WiFi, ESP-NOW, Event transport, listeners and command infrastructure.
- No released version number was changed.

## 2026-08-25 — Issue #40 — Asynchronous serial Command ingress

- Changed transport-style `CommandConsole` line interception from synchronous `CommandRegistry::Invoke()` to transport-neutral `CommandRequestEnvelope` submission through `InboundCommandEvent`.
- Added opaque request IDs and a lifetime-safe registered `ICommandResponseRoute`; completion can therefore return to the serial output after the original interceptor call stack has unwound.
- Preserved interceptor chaining by tokenizing/resolving the root Command before accepting a line, without executing the Command as part of that decision.
- Preserved `CommandConsole::Execute()` as the explicit synchronous local/direct execution API.
- Shutdown now unregisters/detaches the response route before clearing console/registry references so late completion cannot retain a dangling `CommandConsole` pointer.
- Added deterministic host regression coverage proving that serial ingress queues but does not synchronously execute the application Command, delayed completion routes correctly, direct `Execute()` remains synchronous, legacy Console commands still chain, and the route becomes unavailable after shutdown.
- No released version number was changed.

Commits:
- `5226aaf` — `feat(#40): route serial Command ingress through asynchronous Event envelopes`
- `2b4688d` — `test(#40): add deterministic asynchronous Command Event stub`
- `a7095d9` — `test(#40): prove serial ingress queues without synchronous Command execution`

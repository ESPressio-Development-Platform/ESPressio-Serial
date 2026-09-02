# Extending ESPressio Serial

Serial extensions should remain downstream adapters for operator interaction and diagnostics.

## New byte transports

Do not add native/framework byte drivers to Serial core. Implement/adapt them in the appropriate platform layer to the System IO contracts.

## New console features

Preserve bounded input and keep parsing independent of the concrete byte endpoint. Reuse authoritative registries for domain operations rather than creating terminal-only copies.

## New log sinks

A sink should consume a record and emit/store it through a clear bounded ownership model. Consider blocking behaviour, formatting cost and failure handling explicitly.

## New monitors

Use only the monitored library's public contracts. Keep the integration optional and avoid private/native implementation types.

Never expose credentials or protected material merely because a source API can technically provide it.

## Thread/lifecycle changes

Serial's active architecture uses explicit ESPressio Thread lifecycle semantics where worker execution is required. Do not reintroduce historical implicit/free-on-terminate ownership patterns or direct native task ownership when the Threads/Task/System stack already supplies the required abstraction.

## Tests

Extensions should cover the relevant combination of:

- bounded input/output behaviour;
- malformed/oversized diagnostic payloads;
- partial byte reads/writes;
- sink failure;
- registration/observer lifetime;
- concurrent logging/monitor activity;
- absence of optional domain dependencies when integrations are unused;
- secret/credential non-disclosure;
- platform-neutral compilation using test byte endpoints.
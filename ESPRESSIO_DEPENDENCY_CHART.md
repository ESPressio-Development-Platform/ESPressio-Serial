# ESPressio Dependency Chart — Serial 0.5.2

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.svg)

## ESPressio Serial 0.5.2

The Serial core and generic `Console` have no mandatory ESPressio dependencies.
All ESPressio integrations remain opt-in.

### Current integration baselines

```text
CommandConsole / CommandMonitor
    - - -> ESPressio Command >= 0.3.0 < 1.0.0

SecurityMonitor
    - - -> ESPressio Security >= 0.2.0 < 1.0.0

SocketWorkerMonitor
    - - -> ESPressio Sockets >= 0.5.0 < 1.0.0

SocketSecuritySessionMonitor
    - - -> ESPressio Sockets >= 0.5.0 < 1.0.0
    - - -> ESPressio Security >= 0.2.0 < 1.0.0

ESPNowTransportMonitor
    - - -> ESPressio ESP-Now >= 0.5.3 < 1.0.0

SystemClockMonitor
    - - -> ESPressio Timing >= 2.2.4 < 3.0.0

ThreadMonitor
    - - -> ESPressio Threads >= 3.1.4 < 4.0.0

EventMonitor / EventConsole
    - - -> ESPressio Event >= 5.8.4 < 6.0.0
    - - -> ESPressio Serializable >= 0.10.2 < 1.0.0
```

Serial 0.5.2 retains Serializable 0.10.2's bounded, allocation-free ESPB
traversal API for structured Event diagnostics while advancing the validated
Event and ESP-Now baselines to the completed reliability cascade.

## Current coordinated ecosystem

```text
FOUNDATIONAL
├── Observable 3.0.1
├── Serializable 0.10.2
├── Units 0.2.3
├── Security 0.2.0
└── Command 0.3.0

RUNTIME
└── Timing 2.2.4
    ├── Units >= 0.2.3 < 1.0.0
    └── Observable >= 3.0.1 < 4.0.0

EXECUTION
└── Threads 3.1.4
    ├── Timing >= 2.2.4 < 3.0.0
    └── Observable >= 3.0.1 < 4.0.0

TRANSPORT / INTEGRATION
├── Sockets 0.5.0
└── ESP-Now 0.5.3

EVENT
└── Event 5.8.4
    ├── Threads >= 3.1.4 < 4.0.0
    ├── Timing >= 2.2.4 < 3.0.0
    ├── Observable >= 3.0.1 < 4.0.0
    ├── Serializable >= 0.10.2 < 1.0.0 [optional]
    └── ESP-Now >= 0.5.3 < 1.0.0 [optional bridge validation]

DIAGNOSTICS / OPERATOR
└── Serial 0.5.2
```

## Dependency-direction rule

Serial is deliberately a terminal/downstream integration layer. It may observe
or operate against Command, Security, Sockets, ESP-Now, Timing, Threads, Event,
and Serializable, but none of those libraries should acquire a Serial
dependency.

The wider ecosystem should follow the same rule: dependency edges cascade
downstream and integration code belongs with the component that introduces the
additional dependency.

### Known circular optional relationships

Two existing Event bridge placements retain reciprocal optional relationships:

```text
Sockets - - -> Event
    concrete socket Event transports

Event - - -> Sockets
    SocketWorkerEventBridge
    SocketSecuritySessionEventBridge
```

and:

```text
ESP-Now - - -> Event
    ESPNowEventTransport

Event - - -> ESP-Now
    ESPNowTransportEventBridge
```

The optimal longer-term resolution is to keep Event transport-neutral and
relocate transport-specific Observer-to-Event bridges downstream into the
corresponding Sockets/ESP-Now Event integration, or into dedicated integration
packages.

Generic Event bridges for upstream libraries that do not themselves consume
Event—such as Timing, Threads, Command, and Security—do not create this cycle.

## Current Event / ESP-Now cascade

ESP-Now 0.5.3 contains the peer-liveness reliability correction validated with
two ESP32 devices. Event 5.8.4 validates its optional ESP-Now bridge against
that released baseline while retaining the Event 5.8.3 allocation-free
lifecycle synchronization fix. Serial 0.5.2 sits downstream of both and
validates its Event integrations against Event 5.8.4 and its ESP-Now monitor
against ESP-Now 0.5.3.

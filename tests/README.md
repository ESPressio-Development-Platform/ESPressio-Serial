# ESPressio Serial host tests

The host-side test suite currently validates the dependency-free generic `Console` infrastructure.

It exercises:

- command registration and dispatch;
- argument preservation;
- multiple simultaneous interactive line interceptors;
- interceptor removal;
- polling from a `Stream`.

`EventConsole` additionally depends on ESPressio Event 5.6.0, ESPressio Serializable 0.9.0 and the optional ArduinoJson adapter. Its full runtime behaviour is demonstrated by the repository EventConsole examples and is compile-validated against those public APIs during release preparation.


The `EventConsoleContract` test uses narrow test doubles for the Event 5.6 runtime descriptor/factory contract and Serializable JSON adapter. It validates:

- runtime Event listing and description;
- allow-list enforcement;
- JSON command parsing;
- confirmation before dispatch;
- successful type-erased dispatch.

Release preparation also performs compile-oriented validation of `EventConsole` against the real Event 5.6 public API contract.

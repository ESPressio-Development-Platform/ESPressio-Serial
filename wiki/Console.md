# Console

`Console` is the portable interactive operator surface in ESPressio Serial.

It owns:

- bounded line collection;
- optional input echo;
- prompt rendering;
- line interception;
- command registration;
- help and error output.

## Initialization

With a bidirectional byte stream:

```cpp
ESPressio::Serial::Console console;
console.Initialize(consoleIO);
```

With independent endpoints:

```cpp
console.Initialize(input, output);
```

## Bounded input

Line collection is intentionally bounded for embedded reliability. Do not redesign console input around unbounded accumulation of operator text.

Applications should choose command syntax and maximum line sizes appropriate to their device rather than assuming desktop-terminal resources.

## Interception and commands

Console can route complete operator lines into registered handlers or an interception path. Higher-level integrations such as `CommandConsole` reuse this terminal surface rather than implementing another byte parser.

## Prompts and echo

Prompt/echo behaviour belongs to the operator experience and is therefore a Serial concern. The underlying System byte endpoint remains unaware of lines, prompts or commands.

## Concurrency

Treat the console as an owned interactive endpoint. If multiple subsystems emit output concurrently, coordinate them through the logging/console facilities rather than allowing unrelated code to write arbitrary fragments directly to the same native stream.
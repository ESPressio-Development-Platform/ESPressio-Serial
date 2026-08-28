# Diagnostic Safety

Diagnostics must not create a new reliability or security problem while attempting to explain an existing one.

## Credentials and secrets

Monitors should expose operational state, not secret configuration.

For example, WiFi diagnostics may report SSID, RSSI, channel, IP address and station identity while deliberately providing no API to print plaintext passwords.

Apply the same rule to keys, tokens, protected payloads and Security material.

## Bounded parsing and output

Diagnostic decoding must have explicit limits. Malformed or unexpectedly large structured payloads should produce a bounded failure/fallback representation rather than unbounded traversal or allocation.

## Failure containment

A diagnostic formatter should degrade gracefully when it cannot understand a record. Diagnostic code should not crash the application because a payload is malformed.

## Timing-sensitive contexts

Avoid large formatting/output operations directly inside timing-sensitive Observer callbacks. Capture the minimum safe information and defer expensive rendering where the source contract permits it.

## Production logging

Treat log level and monitor enablement as operational policy. Verbose diagnostic surfaces that are useful in a laboratory may be inappropriate in production due to bandwidth, timing, memory or information-disclosure costs.

## Domain ownership

Serial must not bypass Security, Command authorization, Event validation or other source-domain safeguards merely because the caller is physically connected to a console.
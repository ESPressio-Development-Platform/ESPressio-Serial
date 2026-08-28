# Command and Event Consoles

Serial provides optional operator surfaces for ESPressio Command and Event without becoming the authoritative owner of either domain.

## `CommandConsole`

`CommandConsole` integrates the portable console with the shared Command registry:

```text
operator
   |
   v
byte stream
   |
   v
Serial Console
   |
   v
CommandRegistry
   |
   v
domain Command handler
```

Domain handlers remain registered through ESPressio Command. Serial does not maintain a second command truth merely for terminal use.

This means authorization, argument metadata and routing semantics remain consistent with non-Serial command invocation.

## `EventConsole`

`EventConsole` exposes runtime discovery/composition/dispatch for registered Serializable Events. It reuses Event's normal registration, validation and authorization mechanisms.

Do not create terminal-only Event definitions or bypass Event Transport/Serializable validation merely because input originated from an operator.

## Optional dependencies

These integrations are opt-in. A project using only console/logging should not need Command or Event/Serializable simply because those integrations exist in the Serial package.

## Design rule

Serial is an adapter from human/operator text to authoritative domain APIs. It should never become a shadow registry or alternative lifecycle model for those domains.
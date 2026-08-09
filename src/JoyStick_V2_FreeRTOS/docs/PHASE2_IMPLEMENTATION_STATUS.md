# Phase 2 Implementation Status

Phase 2 adds the motor-link protocol and software authorization model without enabling the motor link.

That is the entire deal.

## Implemented in this phase

- fixed 32-byte Drive Command frame;
- fixed 32-byte Drive Status frame;
- little-endian field encoding/decoding;
- CRC-16/CCITT-FALSE;
- explicit protocol version and message types;
- explicit source/destination node addressing for the daisy-chain-capable RS-485 topology;
- broadcast drive commands rejected;
- command-session identity;
- controller-session identity;
- command sequence acknowledgment;
- strict reserved-bit/flag validation;
- bounded parser with no packed-struct casting;
- motor-link state model;
- link fault history exposed through `AppMotorLinkStatus` for debugger/diagnostic visibility;
- repeated-frame link qualification;
- controller restart detection;
- stale acknowledgment rejection;
- session mismatch rejection;
- link timeout handling;
- remote-fault handling;
- software-only logical authorization model;
- explicit authorization blocking reasons;
- physical command forced to zero/unauthorized;
- host fake motor controller;
- fake-controller fault injection;
- deterministic malformed-frame campaign;
- AddressSanitizer/UndefinedBehaviorSanitizer host target;
- GCC `-fanalyzer` static-analysis target;
- Phase-2 invariant checker.

## Still deliberately disabled

- UART5 motor link;
- MAX3535 physical transmit path;
- any non-zero physical drive command;
- actual motor-controller traffic;
- automatic drive authorization in the STM32 task graph.

The current board-level PE7/PE8 conflict still exists. Phase 2 does not make it disappear because a protocol parser compiles.

## Session-ID source

The protocol semantics for session IDs are implemented and tested.

A production source for the joystick controller's per-boot session ID is **not** selected yet because the physical transport is still disabled and persistent configuration/boot identity work belongs in a later phase.

Host tests use explicit deterministic session IDs so every test is reproducible.

## Feedback fields

The V1 status frame reserves signed Q15 left/right measured values.

Those fields are structurally implemented but their final physical meaning is intentionally not frozen yet. The motor-controller feedback contract needs review against the actual controller hardware and control-loop requirements first.

## Validation categories

### Implemented

Yes: protocol, parser, link state, simulator, logical authorization and fail-closed physical boundary.

### Software validated

The acceptance script runs:

```text
Phase-1 invariants
Phase-2 invariants
Phase-1 host tests
Phase-2 host tests
ASan + UBSan host tests
GCC -fanalyzer
STM32 Debug build
STM32 Release build
```

The final target build results are recorded when this phase is applied on the development machine.

### Hardware validated

No.

That category remains empty until the board exists.

## Telemetry boundary

Phase 2 does not define wheel-speed/current/temperature telemetry fields yet. The V1 Drive Status frame is kept focused on the safety handshake. Telemetry gets its own message definition once the motor-controller control-loop units and validity rules are actually specified.

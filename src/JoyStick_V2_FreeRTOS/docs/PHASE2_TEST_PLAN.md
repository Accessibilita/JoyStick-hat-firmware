# Phase 2 Test Plan

Phase 2 is a software-only milestone.

That means the acceptance gate has to be much stronger than "it compiles," but it also has to stop short of claims that require probes, a transceiver, a motor controller, or a chair.

## Build gates

Required before a Phase-2 commit is accepted:

```text
make phase2-check
make host-test
make host-sanitize
make host-analyze
make debug
make release
```

Our code is still compiled with the GhostPCB/MISRA-derived warning policy and warnings remain errors for application-owned source. Phase-2 portable code also runs through GCC `-fanalyzer` as a separate static-analysis gate.

## Protocol tests

### Golden CRC

The standard CCITT-FALSE vector must remain:

```text
"123456789" -> 0x29B1
```

### Round-trip encoding

Encode and decode both command and status frames and prove every field survives unchanged.

### Corruption

Reject at minimum:

- bad CRC;
- wrong magic;
- wrong protocol version;
- wrong message type;
- non-zero reserved bytes;
- unknown flag bits;
- invalid, broadcast, or self-addressed drive frames;
- invalid session values;
- out-of-range defined fields.

## Link-state tests

Exercise:

- first valid response starts synchronization;
- repeated valid traffic reaches READY;
- logical authorization plus accepted command can reach ACTIVE in the software model;
- status from the wrong source/destination address is rejected;
- wrong command-session echo is rejected;
- stale acknowledgment is rejected, including the first frame of a new session;
- motor-controller session change forces resynchronization;
- timeout returns the link to OFFLINE;
- remote fault produces FAULT;
- corrupted frames invalidate current qualification.

## Authorization tests

Prove that logical authorization requires all modeled prerequisites:

- configuration valid;
- input valid;
- mandatory tasks healthy;
- motor link valid and fresh;
- remote controller ready;
- brakes confirmed;
- no remote faults;
- acceptable safety state;
- enable request present;
- command fresh;
- command range valid;
- no local active faults.

Then prove the more important Phase-2 invariant:

```text
logical_authorized == true
```

is still allowed to coexist with:

```text
physical_command.drive_authorized == false
physical forward == 0
physical turn == 0
```

## Malformed-frame campaign

The host test sends 10,000 deterministic pseudo-random 32-byte frames through both decoders.

The point is not to expect any of them to be valid.

The contract is:

> arbitrary bytes may produce a validated frame or a defined error result, but they may not produce an out-of-bounds access, undefined behavior, crash, or unbounded parser path.

The same campaign runs under AddressSanitizer and UndefinedBehaviorSanitizer.

## What this plan does not validate

Not part of Phase 2:

- UART baud/timing;
- MAX3535 enable timing;
- differential signal integrity;
- termination behavior;
- real motor-controller interoperability;
- real controller reboot timing;
- real packet-loss rate;
- wheel-speed scale;
- physical braking behavior;
- motor timeout behavior on actual hardware.

Those stay on the hardware-validation debt list until the hardware exists.

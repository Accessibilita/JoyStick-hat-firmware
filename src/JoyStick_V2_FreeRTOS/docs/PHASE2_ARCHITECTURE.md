# Phase 2 Architecture

Phase 1 built the safety boundary.

Phase 2 builds the motor-link machinery up to that boundary without crossing the physical one.

That gives `experimental` somewhere useful to go while the target hardware is unavailable: we can keep proving software behavior instead of either freezing development or pretending host tests are bench tests.

## The new split

Phase 2 formalizes three different things that are easy to accidentally collapse into one:

```text
RequestedDriveCommand
        |
        v
Logical authorization
        |
        v
Physical AuthorizedDriveCommand
        |
        v
Motor-link transport
```

During Phase 2, logical authorization is allowed to become true in host tests.

The physical command is still forced to:

```text
forward = 0
turn = 0
maximum speed = 0
drive_authorized = false
```

That is not contradictory. It is the whole point of the phase.

We can prove the decision logic without creating a path that can move hardware.

## New modules

```text
App/Inc/motor_protocol.h
App/Src/motor_protocol.c
    Fixed 32-byte command/status protocol
    Explicit little-endian field encoding
    CRC-16/CCITT-FALSE
    Strict frame validation

App/Inc/motor_link_state.h
App/Src/motor_link_state.c
    Source/destination address validation
    Session qualification
    Exact command-sequence acknowledgment
    First-frame stale-ack rejection
    Controller restart detection
    Ready/active/offline/fault model
    Timeout handling

App/Inc/command_authorization.h
App/Src/command_authorization.c
    Software-only logical authorization model
    Explicit blocking reasons
    Hard physical-command inhibition

tests/host/fake_motor_controller.*
    Deterministic motor-controller simulator
    Fault injection without target hardware
```

## Protocol path

```text
Requested drive state
        |
        v
CommandAuthorization_Evaluate()
        |
        +---- logical decision
        |
        +---- physically inhibited command

logical decision + status
        |
        v
MotorProtocol command frame
        |
        v
Fake motor controller
        |
        v
MotorProtocol status frame
        |
        v
MotorLinkState
        |
        v
AppMotorLinkStatus
```

The fake controller exists because protocol bugs are much cheaper when they happen in a process on Linux than when they happen between two powered embedded boards.

`AppMotorLinkStatus` is expanded in Phase 2 so the debugger can see the controller session, last acknowledged command, local/remote node addresses, protocol state, command-accepted flag, and accumulated local link-fault history without teaching the safety task to parse protocol internals.

## What the simulator can do

The host fake controller supports deliberate bad behavior:

- drop a response;
- corrupt the CRC;
- return traffic from the wrong node;
- echo the wrong command session;
- restart into a new controller session;
- acknowledge a stale sequence;
- report a remote fault;
- report itself not ready.

That gives us a repeatable fault campaign rather than a collection of one-off manual tests.

## Parser rules

The frame decoder is intentionally boring.

It does not:

- allocate memory;
- cast byte arrays to packed structs;
- trust a remote length field;
- perform unbounded scans;
- accept unknown flag bits;
- accept non-zero reserved bytes;
- accept zero/unassigned or broadcast drive addresses;
- accept zero session IDs;
- accept bad CRCs and "see what happens."

A frame either becomes a validated typed message or it produces a defined decode error.

## Hardware boundary

The Phase-1 physical lock remains intact.

`rs485_link_task.c` still keeps the current transceiver path disabled and publishes an invalid physical link.

The new protocol/state-machine code is therefore a software model and future transport layer, not a hidden UART enable.

That lets target Debug/Release builds compile the exact code we are testing without claiming the board has executed it.

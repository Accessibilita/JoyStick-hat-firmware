# Motor Link Protocol V1

Phase 2 finally gives the joystick controller and the drive-motor controller a real contract instead of a vague promise that "RS-485 will carry commands later."

The important design choice is that this protocol is defined **before** the physical link is enabled.

That lets us beat on framing, addressing, sequencing, session handling, stale data, controller restarts, bad CRCs, and authorization behavior on a host machine while the actual MAX3535 transmitter is still hard-disabled in firmware.

## Where this came from

This is not a clean-sheet protocol.

Two older Accessibilita designs fed into it:

- the earlier RS-485 actuator/recline controller used explicit start/address/command/length/checksum/end framing;
- the later motor-controller communications draft used fixed 32-byte request/response frames, little-endian fields, protocol versioning, sequence numbers, and CRC-16/CCITT-FALSE.

Phase 2 keeps the stronger fixed-frame/CRC/session ideas and brings back explicit node addressing because the drive-motor-controller hardware is designed around an RS-485 interface with daisy-chain capability.

The old XOR checksum does not come along for the ride.

## Transport model

The V1 protocol uses fixed 32-byte addressed frames:

```text
Joystick / host node                     Drive controller node
        |                                         |
        |------ Drive Command, 32 bytes --------->|
        |                                         |
        |<------ Drive Status, 32 bytes -----------|
        |                                         |
```

On a daisy-chained physical network, other nodes may observe the traffic. Every frame therefore names both its source and destination.

A Drive Command and Drive Status frame are always unicast in V1. Broadcast motion commands are deliberately invalid.

The fixed frame size is deliberate. There is no variable-length parser, no heap allocation, and no need for the receiver to trust a length field supplied by another node.

The transport layer may later use UART DMA, timeout-driven receive completion, or another bounded implementation. That physical binding is not part of Phase 2.

## Addressing

One byte is reserved for the source node and one for the destination node.

```text
0x00       unassigned / invalid
0x01-0xFE  valid unicast node address
0xFF       broadcast / reserved
```

Drive Command and Drive Status frames require two different valid unicast addresses.

That means:

- a node may not send a drive command to `0x00`;
- a node may not broadcast a drive command to `0xFF`;
- source and destination may not be the same address;
- link state rejects status from the wrong node even if its CRC and session fields are otherwise perfect.

The final production address allocation is not frozen in Phase 2. Host tests use deterministic addresses only so the behavior is reproducible.

Addressing is an application-layer decision only. Phase 2 does not claim that bus arbitration, forwarding through a physical daisy chain, turnaround timing, termination, or collision behavior has been solved. Those require the actual motor-controller schematic/topology and hardware validation.

## Byte order

All multi-byte integer fields are little-endian.

Wire bytes are encoded and decoded field-by-field. The firmware does not cast a receive buffer onto a packed C struct.

That avoids making protocol correctness depend on compiler padding, alignment, aliasing behavior, or host endianness.

## CRC

Both frames use CRC-16/CCITT-FALSE over bytes 0 through 29.

```text
Width:       16
Polynomial:  0x1021
Init:        0xFFFF
RefIn:       false
RefOut:      false
XorOut:      0x0000
Check:       "123456789" -> 0x29B1
```

The CRC itself is stored little-endian in bytes 30 and 31.

## Drive Command frame

Magic bytes retain the request-side magic from the earlier fixed-frame motor-controller work:

```text
A5 5A
```

| Offset | Size | Field | Meaning |
|---:|---:|---|---|
| 0 | 1 | Magic 0 | `0xA5` |
| 1 | 1 | Magic 1 | `0x5A` |
| 2 | 1 | Version | Protocol version, currently `1` |
| 3 | 1 | Message type | `0x10` = Drive Command |
| 4 | 1 | Destination | Drive-controller node address |
| 5 | 1 | Source | Joystick/host node address |
| 6 | 1 | Command flags | Logical authorization / enable request |
| 7 | 1 | Safety state | Current joystick-controller safety state |
| 8 | 4 | Command session ID | Non-zero ID for the joystick-controller protocol session |
| 12 | 4 | Sequence | Command transaction sequence |
| 16 | 4 | Generated time | Sender-local command generation time in milliseconds |
| 20 | 2 | Forward | Signed Q15 forward request |
| 22 | 2 | Turn | Signed Q15 turn request |
| 24 | 2 | Maximum speed | Unsigned Q15 speed ceiling |
| 26 | 4 | Active faults | Local joystick-controller fault mask |
| 30 | 2 | CRC | CRC-16/CCITT-FALSE over bytes 0–29 |

`Generated time` is sender-local diagnostic/freshness metadata. It is not a synchronized cross-controller clock and the motor controller must not treat it as one.

### Command flags

```text
bit 0  logical authorization is active
bit 1  enable request is active
bits 2-7 reserved, must be zero
```

A logically authorized command is still not a physical drive command in Phase 2.

That distinction is intentional and tested.

## Drive Status frame

The response magic also comes directly from the earlier fixed-frame protocol draft:

```text
4D 43    ASCII "MC"
```

| Offset | Size | Field | Meaning |
|---:|---:|---|---|
| 0 | 1 | Magic 0 | `0x4D` |
| 1 | 1 | Magic 1 | `0x43` |
| 2 | 1 | Version | Protocol version, currently `1` |
| 3 | 1 | Message type | `0x20` = Drive Status |
| 4 | 1 | Destination | Joystick/host node address |
| 5 | 1 | Source | Drive-controller node address |
| 6 | 1 | Status flags | Link/drive/brake/acceptance state |
| 7 | 1 | Status code | Explicit response disposition |
| 8 | 4 | Controller session ID | Non-zero drive-controller boot/session identity |
| 12 | 4 | Command session echo | Joystick session this response belongs to |
| 16 | 4 | Acknowledged sequence | Command transaction this response belongs to |
| 20 | 4 | Controller uptime | Drive-controller milliseconds since boot |
| 24 | 4 | Remote faults | Drive-controller fault mask |
| 28 | 2 | Reserved | Must be zero in V1 |
| 30 | 2 | CRC | CRC-16/CCITT-FALSE over bytes 0–29 |

The V1 safety/status response intentionally does **not** try to cram wheel velocity, current, temperature, or other telemetry into the last few bytes just because space exists.

Those quantities need defined units, update rates, validity rules, and fault semantics. They deserve a separate telemetry message type once the motor-controller control loop is reviewed instead of becoming mystery fields in the safety handshake.

## Status flags

```text
bit 0  link ready
bit 1  drive subsystem ready
bit 2  brakes confirmed
bit 3  command accepted
bits 4-7 reserved, must be zero
```

## Status codes

```text
0  OK
1  inhibited
2  remote fault
3  bad session
4  bad sequence
5  bad command
6  unsupported version
```

A zero command with `OK` is not the same thing as an invalid link, and an invalid link is not the same thing as revoked authorization.

Those states remain separate all the way through the protocol model.

## Session rules

There are two session concepts:

- **command session ID** belongs to the joystick controller;
- **controller session ID** belongs to the drive controller.

The drive controller echoes the current command session in every status frame.

The joystick controller learns the drive-controller session and requires it to remain stable. If the controller session changes, the remote controller has restarted and the link drops back into synchronization.

A packet from the previous command session is stale even when its CRC is perfect.

A status packet from the previous controller boot is stale even when its sequence looks reasonable.

This is why session identity exists separately from sequence numbers.

The session semantics are implemented in Phase 2. The production source for per-boot session IDs is intentionally left for later configuration/boot-identity work while the physical link is disabled.

## Sequence rules

Each Drive Status frame acknowledges exactly one Drive Command sequence.

Phase 2 requires the response to acknowledge the command transaction currently being evaluated. A stale or unexpected acknowledgment does not quietly become the new link state.

That rule also applies to the first response of a newly observed controller session. A new session ID does not grant stale data a free pass.

The first implementation is strict because loosening a protocol later is easier than proving that a permissive parser was always safe.

## Link qualification

One valid frame is not enough to declare the motor link healthy.

The Phase-2 state model requires repeated valid traffic before moving from synchronization to ready.

```text
OFFLINE
   |
   v
SYNCHRONIZING
   |
   v
READY
   |
   v
ACTIVE
```

A wrong node address, controller restart, session mismatch, acknowledgment mismatch, malformed frame, timeout, or remote fault moves the model away from ACTIVE.

Traffic returning after a failure does not magically restore authorization. The link has to qualify again.

## Physical link status

The current joystick PCB still has the reviewed PE7/PE8 UART direction conflict.

Phase 2 does **not** enable UART5 and does **not** transmit these frames on the board.

The protocol is compiled into the target firmware and exercised on the host, but the physical transport remains blocked by:

```text
APP_RS485_PHYSICAL_LINK_ENABLE == 0
```

That line stays between "we designed and tested a protocol" and "we put bytes on a safety-relevant wire."

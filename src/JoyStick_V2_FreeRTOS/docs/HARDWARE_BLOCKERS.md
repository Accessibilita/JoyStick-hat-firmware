# Hardware Blockers

Firmware does not get to vote inconvenient hardware facts out of existence.

These are the current board-level issues that directly affect the firmware architecture.

## HB-001 — RS-485 / UART direction conflict

The reviewed board connects the MAX3535 transmit input `DI` to STM32 PE7 and the receiver output `RO1` to PE8.

The usable UART5 mapping on those pins goes the other way:

```text
PE7 = UART5_RX
PE8 = UART5_TX
```

So the board-level signal direction and the MCU peripheral direction are crossed.

That is a real hardware conflict.

### Phase-1 response

Phase 1 refuses to pretend otherwise.

- PE7 is held as a safe GPIO output.
- PE8 is treated as an input.
- PC9 / MAX3535 DE is held low.
- the receiver-enable line is placed in its reviewed safe state.
- UART5 is not initialized for the motor link.
- `APP_RS485_PHYSICAL_LINK_ENABLE` is locked off.

The system therefore cannot acquire drive capability because somebody happened to enable a UART in CubeMX.

### Actual fixes

The eventual repair needs to exist in hardware or in a deliberately documented board modification.

Candidates include rerouting/bodge-wiring the interface to a valid UART pin pair or fixing the next PCB revision.

Whatever we choose needs to show up in the schematic, netlist, firmware pin map, and hardware revision record.

## HB-002 — Single-channel joystick fault ambiguity

Each joystick axis is one potentiometer channel.

That is enough to measure position.

It is not enough to prove that every measured endpoint represents a healthy joystick.

Some wiring failures can produce voltages that are electrically indistinguishable from valid full-scale commands.

The existing 1 MΩ pull-up and 0.1 µF capacitor also give an open wiper a long enough decay path that it can move through apparently plausible values rather than instantly becoming an obvious fault.

### Phase-1 response

Firmware currently checks:

- ADC range
- acquisition freshness
- DMA/acquisition health
- regulator power-good
- neutral qualification

Those checks are useful.

They are not redundant joystick hardware.

Phase 1 remains drive-inhibited and makes no claim that single-fault joystick coverage is complete.

A future hardware revision should evaluate complementary/redundant joystick channels, controlled joystick power, and independent supply monitoring.

## HB-003 — Motor-controller safety contract

The joystick interface cannot define a complete safe state by itself.

The motor controller needs a specified contract covering at least:

- command timeout
- what happens when communication disappears
- brake behavior
- current/torque limiting
- measured motion feedback
- session/restart behavior
- sequence handling
- reauthorization after a fault

Until that exists, "send zero" and "stop transmitting" are not interchangeable design decisions.

### Phase-1 response

The motor link is considered invalid.

The safety state machine cannot enter drive authorization.

The published authorized command stays neutral and inhibited.

That keeps the unresolved interface on the correct side of the safety boundary: outside it.

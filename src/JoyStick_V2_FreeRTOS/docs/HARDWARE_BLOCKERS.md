# Hardware blockers and assumptions

## HB-001 — RS-485 UART direction mismatch

The reviewed KiCad netlist connects MAX3535 pin 25 (`DI`) to STM32 PE7 and MAX3535 pin 28
(`RO1`) to STM32 PE8. The existing Cube configuration assigns PE7 to `UART5_RX` and PE8 to
`UART5_TX`. The receive and transmit directions are therefore crossed at the PCB level.

Phase 1 response:

- PE7 is configured as a low GPIO output so the disabled transceiver input is not floating.
- PE8 is configured as a GPIO input.
- PC9, connected to MAX3535 `DE`, is held low.
- PC8, connected to MAX3535 receiver-enable, is held low.
- No UART peripheral is enabled for the motor link.
- `APP_RS485_PHYSICAL_LINK_ENABLE` is compile-time locked to zero.

Potential hardware corrections include bodge-wiring the transceiver to a valid TX/RX pair such
as UART5 on PC12/PD2, or revising the next PCB. The selected repair must be documented in a new
hardware revision and reflected in the KiCad netlist and `.ioc` pin map.

## HB-002 — Single-channel joystick diagnostics

Each joystick axis is a single potentiometer channel. Several cable faults can be electrically
indistinguishable from a legitimate full-scale command. The 1 MΩ pull-up and 0.1 µF capacitor
also create an approximately 100 ms time constant after an open wiper.

Phase 1 response:

- The software checks engineering bounds and freshness.
- The build remains drive-inhibited.
- No claim is made that single-fault joystick diagnostics are complete.

A production hardware revision should evaluate redundant/complementary joystick channels,
current-limited joystick power, and independent supply monitoring.

## HB-003 — Motor-controller safety contract undefined

A complete safe state requires a jointly specified motor-controller protocol, command timeout,
brake behavior, measured velocity feedback, and reauthorization handshake. Those requirements
are not yet implemented or verified.

Phase 1 response:

- The link task always publishes `link_valid = false`.
- The safety state machine cannot reach `APP_SAFETY_DRIVE_AUTHORIZED`.
- The authorized command mailbox contains neutral/inhibited commands only.

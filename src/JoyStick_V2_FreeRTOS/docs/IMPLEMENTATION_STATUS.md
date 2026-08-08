# Phase 1 Implementation Status

Phase 1 has crossed the line from generated scaffold to real firmware.

It is still intentionally incapable of driving the chair.

Those two statements are both important.

## Working now

The current tree implements:

- conventional STM32CubeIDE project metadata plus a project-owned GNU Makefile
- STM32F446VET6 startup and peripheral foundation
- 96 MHz HSE/PLL system clock
- four statically allocated application tasks
- statically allocated queues/mailboxes
- dynamic application allocation disabled
- TIM2-triggered 1 kHz ADC1 scan
- PA0 and PA1 joystick acquisition
- circular DMA
- five samples per axis per safety batch
- DMA half/full completion notification
- sequence tracking and change-while-copying detection
- acquisition freshness checking
- ADC error/overrun tracking
- joystick range diagnostics
- regulator 3.3 V PG observation
- neutral qualification
- safety-state-machine foundation
- explicit `AuthorizedDriveCommand`
- one-element overwrite command mailbox
- mandatory-task health supervision
- Safety-Control-owned IWDG refresh
- debugger watchdog freeze in Debug builds
- safe LED blanking
- forced-disabled RS-485 driver
- debugger snapshot `g_app_phase1_debug_snapshot`
- host-side tests
- source/project invariant checks

## Deliberately absent

Phase 1 does not contain:

- motor command transmission
- a production RS-485 protocol
- a valid drive authorization state
- persistent runtime calibration
- production joystick response curves
- production speed profiles
- finished button/rotary interpretation
- finished LED protocol
- USB/service behavior
- production motor-controller synchronization
- an argument that this firmware is ready to move a person

## Build status

The target firmware has now been built successfully from the actual Git checkout using:

```text
STM32CubeIDE 2.2.0
GNU Tools for STM32 14.3.rel1
arm-none-eabi-gcc 14.3.1
```

Validated:

```text
make phase1-check    PASS
make debug           PASS
make release         PASS
```

Debug and Release generate ELF, Intel HEX, and raw BIN images.

The firmware dependencies have also been pulled back to the exact STM32CubeF4 V1.28.1 baseline used by the original project.

That gives us a reproducible combination of a modern development toolchain and a controlled historical firmware platform.

## What is not validated

The physical board is not currently available for acceptance testing.

So we are **not** claiming validation of:

- startup on the STM32F446
- actual ADC voltage/count behavior
- DMA timing on silicon
- joystick noise and real mechanical center
- open/short fault behavior
- 3.3 V PG timing
- watchdog reset behavior on target
- debugger snapshot coherence on target
- GPIO startup levels measured at the board
- physical RS-485 behavior
- motor-controller integration

A build is evidence that the software is internally coherent enough to become a binary.

It is not evidence that the complete electromechanical system is safe.

## Remaining software cleanup

Known non-fatal cleanup includes:

- explicitly define the HAL SPI `USE_SPI_CRC` setting
- provide deliberate newlib syscall behavior instead of relying on `nosys` warnings
- inspect and eliminate the RWX ELF LOAD-segment warning
- continue extending host-side fault tests
- build the actual RS-485 protocol only after the physical link definition is resolved

## Next useful work

The next meaningful milestone is hardware bring-up.

That means proving the assumptions in this repository against an actual board instead of adding more features on top of assumptions.

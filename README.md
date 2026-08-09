# JoyStick Interface Firmware

Firmware for the Accessibilita joystick interface board, built around the STM32F446VET6.

This repository started life as a mostly Cube-generated FreeRTOS project. The current work is turning that into firmware we can actually reason about: deterministic joystick acquisition, explicit safety state, supervised task health, a real watchdog policy, and a hard boundary between requested motion and authorized motion.

That last part matters enough to say twice:

**Raw joystick data is not a motor command.**

Phase 1 is the safety foundation. The `experimental` branch now carries the software-validated Phase-2 motor-link model and Phase-3 joystick calibration/configuration/shaping model while hardware validation remains pending.

## Where the firmware lives

The STM32 project is here:

```text
src/JoyStick_V2_FreeRTOS/
```

That directory contains the application code, STM32 peripheral layer, vendored firmware dependencies, build system, tests, and the engineering documentation for the board.

## What Phase 1 is

Phase 1 is the concrete slab.

It gives us enough real firmware to bring up the board, inspect the ADC/DMA path, step the system over SWD/JTAG, exercise the RTOS architecture, test the watchdog, and start fault injection without also giving unfinished software a path to powered traction hardware.

The control architecture is intentionally explicit:

```text
Raw inputs
    ↓
Diagnostics
    ↓
Safety observation
    ↓
Safety state machine
    ↓
Requested command
    ↓
Authorization
    ↓
AuthorizedDriveCommand
```

Only Safety Control owns that final authorization step.

In Phase 1, authorization always stops there.

## Current software state

Implemented now:

- STM32F446VET6 platform foundation
- ADC1 scan of both joystick axes
- TIM2-triggered acquisition
- circular DMA
- bounded five-sample acquisition batches
- DMA completion notification into the safety task
- acquisition sequence, age, freshness, and race checks
- joystick electrical plausibility checks
- 3.3 V regulator power-good observation
- explicit safety state machine
- neutral qualification
- static FreeRTOS tasks and mailboxes
- task-health supervision
- IWDG ownership by Safety Control
- debugger-facing application snapshot
- Debug and Release Makefile builds
- fail-closed Phase-1 invariant checks

Not implemented yet:

- motor command transmission
- finished RS-485 protocol
- hardware-backed calibration storage
- hardware-validated production joystick mapping
- final HMI behavior
- production motor-controller safety handshake
- hardware qualification

## The four application tasks

The old generated task-per-peripheral scaffold is gone.

The firmware is organized around responsibilities instead:

```text
Safety Control   highest application priority
RS-485 Link
HMI
Diagnostics      lowest application priority
```

Safety Control owns the actual safety decision.

The other tasks can report information or perform work, but they do not get to grant motion.

## Build baseline

Current development is validated with:

- STM32CubeIDE 2.2.0
- GNU Tools for STM32 14.3.rel1
- `arm-none-eabi-gcc` 14.3.1

The firmware libraries deliberately remain on the original project baseline:

- STM32CubeF4 V1.28.1
- STM32F4 HAL from V1.28.1
- CMSIS Device F4 from V1.28.1
- CMSIS Core from the V1.28.1 package
- FreeRTOS from the V1.28.1 package

The compiler and the firmware package are separate dependencies. There is no reason an IDE upgrade should silently drag the embedded platform underneath the application along with it.

Exact revisions are recorded in `src/JoyStick_V2_FreeRTOS/docs/DEPENDENCIES.md`.

## Build it

From the firmware project directory:

```bash
make phase1-check
make debug
make release
```

Both Debug and Release currently build successfully with the STM32CubeIDE 2.2.0 GNU Arm toolchain.

Generated images land under `build/Debug/` and `build/Release/`. Build products stay out of Git.

## The hardware has opinions

There are several things firmware cannot fix by being clever.

The current PCB routes the MAX3535 transmit and receive signals onto PE7/PE8 in the opposite direction from the available UART5 mapping. Phase 1 does not hide that with a software trick. The UART is not enabled and the physical driver is held disabled.

Each joystick axis is also a single potentiometer channel. That means some electrical failures can look exactly like a legitimate endpoint command. Software can range-check and freshness-check the signal, but it cannot invent hardware redundancy that does not exist.

Those issues are documented because they affect the architecture.

See `docs/HARDWARE_BLOCKERS.md` and `docs/HARDWARE_PINMAP.md`.

## Safety lock

Phase 1 cannot authorize drive motion.

Specifically:

- `drive_authorized` remains false
- authorized forward command remains zero
- authorized turn command remains zero
- the physical RS-485 driver remains disabled
- configuration validity remains false
- the invariant checker looks for accidental removal of those guards

This is deliberate.

Do not connect this Phase-1 firmware to powered traction hardware and assume that a successful compile means the system is ready to move a person.

## Hardware validation

The software build has been validated. The physical board has not.

We still need to verify reset/startup, actual ADC behavior, TIM2/DMA timing, joystick movement and fault injection, regulator PG, watchdog behavior, debugger snapshot behavior, GPIO safe states, and the eventual physical RS-485 path.

There is a very large difference between "the firmware builds" and "the machine has been proven safe on the bench."

We are currently at the first one.

## Documentation

Start here:

- `docs/PHASE1_ARCHITECTURE.md` — how the firmware is put together
- `docs/IMPLEMENTATION_STATUS.md` — what exists and what does not
- `docs/HARDWARE_PINMAP.md` — reviewed MCU-to-board mapping
- `docs/HARDWARE_BLOCKERS.md` — things firmware must not paper over
- `docs/DEPENDENCIES.md` — exact STM32 software baseline
- `docs/CODING_STANDARD.md` — rules for code we own
- `docs/STM32CUBEIDE_WORKFLOW.md` — build/debug workflow
- `docs/CUBEMX_REGENERATION.md` — why Generate Code is not currently authority

This firmware is being built from the safety boundary outward. Motion comes after the system earns it, not before.

## Experimental Phase 2 — motor-link software model

The `experimental` branch is now moving beyond the Phase-1 foundation while the physical board is unavailable.

Phase 2 adds the motor-controller communications contract entirely on the software side:

- fixed 32-byte command/status frames;
- source/destination node addressing for the daisy-chain-capable RS-485 link;
- CRC-16/CCITT-FALSE;
- protocol versioning;
- command and controller sessions;
- sequence acknowledgment;
- link qualification and timeout handling;
- controller-restart detection;
- software-only logical drive authorization;
- a fake motor controller for host fault injection.

The physical RS-485 path is still locked out.

That split is deliberate: `experimental` is allowed to prove more software than we can prove hardware, but the documentation keeps those categories separate.

See:

```text
src/JoyStick_V2_FreeRTOS/docs/MOTOR_PROTOCOL.md
src/JoyStick_V2_FreeRTOS/docs/PHASE2_ARCHITECTURE.md
src/JoyStick_V2_FreeRTOS/docs/PHASE2_TEST_PLAN.md
src/JoyStick_V2_FreeRTOS/docs/PHASE2_IMPLEMENTATION_STATUS.md
```

## Experimental Phase 3 — joystick calibration and command shaping

Phase 3 gives raw joystick ADC data a controlled path into `AppRequestedDriveCommand` without changing the physical drive lock.

The CHC-104B-M2 is the initial reference joystick for bench work. Its name is recorded as a reference profile, but its endpoint counts, center, noise, deadband, direction, and production response curve are **not** invented in software. Those values wait for the real stick and board.

Phase 3 adds:

- bounded center and sweep calibration;
- asymmetric per-axis min/center/max calibration;
- guarded endpoints and explicit deadband;
- signed Q15 normalization;
- optional axis inversion;
- integer linear-to-cubic response shaping;
- Q15 maximum-speed limiting;
- X-to-turn and Y-to-forward requested-command construction;
- fixed 64-byte versioned configuration records;
- CRC32/IEEE record protection;
- redundant two-slot newest-valid selection and torn-write recovery modeling;
- malformed-record and full-ADC-domain host campaigns.

The STM32 flash backend is intentionally not implemented yet. Safety Control also continues to report configuration invalid at runtime, and the Phase-2 physical authorization boundary remains zeroed.

See:

```text
src/JoyStick_V2_FreeRTOS/docs/PHASE3_ARCHITECTURE.md
src/JoyStick_V2_FreeRTOS/docs/CONFIGURATION_FORMAT.md
src/JoyStick_V2_FreeRTOS/docs/CHC104B_M2_REFERENCE.md
src/JoyStick_V2_FreeRTOS/docs/PHASE3_TEST_PLAN.md
src/JoyStick_V2_FreeRTOS/docs/PHASE3_IMPLEMENTATION_STATUS.md
```

## Experimental Phase 4 — runtime integration

Phase 4 connects the Phase-1 safety state, Phase-2 authorization model, and Phase-3 calibration/shaping code into the live application path. It adds debounced HMI state, runtime configuration selection, requested-command generation, and a much richer debugger snapshot.

The target configuration backend and target HMI drive-enable mapping remain intentionally unqualified, and the physical RS-485/drive path remains locked out. See `src/JoyStick_V2_FreeRTOS/docs/PHASE4_ARCHITECTURE.md`.

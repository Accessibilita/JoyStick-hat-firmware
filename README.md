# JoyStick Interface Firmware

Firmware for the Accessibilita joystick interface board.

This repository contains the STM32 firmware that sits between the joystick/HMI hardware and the eventual motor-control link.

The original codebase was largely a Cube-generated FreeRTOS starting point. Current development is replacing that scaffold with an architecture where the important boundaries are explicit: acquisition, diagnostics, safety state, requested motion, and authorized motion are separate things.

That matters because a joystick ADC count should never become a motor command just because enough functions passed it along.

## Active development

The safety-first rewrite is currently being developed on:

```text
agent/phase1-safety-foundation
```

The STM32 project on that branch lives at:

```text
src/JoyStick_V2_FreeRTOS/
```

Phase 1 is deliberately not drive-capable.

It establishes the boring-but-critical foundation first:

- deterministic dual-axis joystick acquisition
- timer-triggered ADC + DMA
- input diagnostics and freshness checking
- explicit safety state
- static FreeRTOS application architecture
- task-health supervision
- watchdog ownership
- debugger visibility
- reproducible firmware dependencies
- a hard authorization boundary before anything can become a drive command

The basic model is:

```text
Raw inputs
    ↓
Diagnostics
    ↓
Safety state
    ↓
Requested command
    ↓
Authorization
    ↓
AuthorizedDriveCommand
```

The current Phase-1 implementation keeps the final authorization inhibited.

## Development baseline

Current Phase-1 work uses:

```text
STM32CubeIDE 2.2.0
GNU Tools for STM32 14.3.rel1
arm-none-eabi-gcc 14.3.1
```

The embedded firmware dependencies remain pinned to the original STM32CubeF4 V1.28.1 project baseline.

Keeping the compiler/tooling and the target firmware package as separate controlled dependencies is intentional.

## Hardware reality

There are known hardware constraints under active review.

The current board has an RS-485/UART direction conflict around the MAX3535 and PE7/PE8 mapping, so the Phase-1 firmware does not enable that physical drive path.

The joystick is also single-channel per axis, which limits the faults that can be distinguished electrically from legitimate endpoint commands.

Those are documented engineering constraints, not things firmware should hide.

## Status

The active Phase-1 branch currently passes its source invariants and clean Debug/Release target builds.

Physical board execution and fault-injection testing are still pending.

A successful build is the point where hardware bring-up can start. It is not a claim that the complete machine has been qualified.

For the current architecture, build instructions, dependency revisions, hardware pin map, blockers, and bring-up notes, switch to `agent/phase1-safety-foundation` and read:

```text
src/JoyStick_V2_FreeRTOS/README.md
src/JoyStick_V2_FreeRTOS/docs/
```

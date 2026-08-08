# JoyStick V2 Firmware

This is the active STM32F446 firmware project for the Accessibilita joystick interface.

Phase 1 is intentionally not drive-capable. The point of this tree is to give us a deterministic, inspectable foundation for real board bring-up before motor control enters the picture.

## What changed

The generated FreeRTOS scaffold has been replaced with an application architecture built around actual responsibilities:

```text
Safety Control
RS-485 Link
HMI
Diagnostics
```

Safety Control owns input validation, safety state, command authorization, task health, and watchdog supervision.

The important boundary is:

```text
raw joystick
    ↓
diagnostics
    ↓
safety state
    ↓
requested motion
    ↓
authorization
    ↓
authorized motion
```

A requested command is not automatically an authorized command.

Phase 1 never authorizes one.

## Acquisition path

The joystick path is currently:

```text
TIM2 @ 1 kHz
    ↓
ADC1
    ├── PA0 / Joystick Y
    └── PA1 / Joystick X
    ↓
circular DMA
    ↓
5-sample completed batch
    ↓
Safety Control Task
    ↓
input diagnostics
    ↓
safety state machine
```

The DMA side records completion sequence information so the task can detect a buffer changing while it is being copied instead of quietly consuming a half-updated sample set.

## RTOS rules

All application tasks and application queues are statically allocated.

The safety path does not:

- allocate memory
- wait on mutexes
- write flash
- format log strings
- block on peripheral I/O
- use a FIFO for motion commands

Motion/state mailboxes use overwrite semantics because the newest state is what matters. Old steering commands do not deserve a waiting room.

## Watchdog

The independent watchdog belongs to Safety Control.

Other tasks report progress. They do not refresh the watchdog themselves.

Safety Control decides whether mandatory application progress is credible enough to keep the machine alive.

Debug builds freeze IWDG while halted under the debugger. Release builds do not.

## Build

Validated development environment:

```text
STM32CubeIDE 2.2.0
GNU Tools for STM32 14.3.rel1
arm-none-eabi-gcc 14.3.1
```

From this directory:

```bash
make phase1-check
make debug
make release
```

Current Debug and Release target builds pass.

The project-owned Makefile is the build authority. STM32CubeIDE uses that same Makefile rather than maintaining a second independent description of the firmware.

## Firmware dependencies

The original project was created against STM32CubeF4 V1.28.1, so that is the firmware baseline retained here.

We reconstructed that exact ST release instead of copying in the latest HAL and calling it close enough.

See `docs/DEPENDENCIES.md` for the exact commits.

## Phase-1 safety lock

This build is deliberately inhibited:

- no motor UART is initialized
- MAX3535 driver enable stays low
- `drive_authorized == false`
- authorized forward command is zero
- authorized turn command is zero
- configuration validity remains false
- source invariants fail if the core guards are removed

That is not unfinished boilerplate. It is an architectural constraint.

## Known hardware blockers

The current PCB has a real UART/RS-485 direction conflict on PE7/PE8.

The joystick also uses one potentiometer channel per axis, which means some single electrical failures cannot be distinguished from valid endpoint commands.

Neither problem gets magically transformed into a software feature.

Read `docs/HARDWARE_BLOCKERS.md` before enabling anything that can move.

## Project layout

```text
App/            application-owned RTOS and safety logic
Core/           MCU initialization and interrupt integration
Platform/       board-level hardware ownership wrappers
Drivers/        STM32 HAL + CMSIS dependency baseline
Middlewares/    FreeRTOS dependency baseline
docs/           architecture and engineering records
tests/host/     portable host-side tests
tools/          invariant and project utility scripts
```

## What is proven

Software-side:

- Phase-1 invariant checker passes
- Debug target build passes
- Release target build passes
- project uses the intended CubeIDE 2.2.0 compiler
- ELF/HEX/BIN images are generated

Hardware-side:

**not proven yet**

No board execution, ADC measurement, watchdog fault injection, JTAG bring-up, or physical RS-485 test is claimed by this phase.

That comes next.

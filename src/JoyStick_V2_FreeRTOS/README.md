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

## Phase 2 on `experimental`

Phase 1 built the inhibited safety foundation. Phase 2 builds the motor-link protocol and authorization model on top of it without enabling the physical motor link.

The new software path is:

```text
Requested drive state
        ↓
logical authorization
        ↓
address validation + 32-byte Drive Command frame
        ↓
fake motor controller / future transport
        ↓
32-byte Drive Status frame
        ↓
link/session/sequence validation
        ↓
AppMotorLinkStatus
```

Host tests are allowed to reach a logically authorized state so we can prove the decision machinery. The actual `AppAuthorizedDriveCommand` returned by the Phase-2 authorization boundary is still forced to zero with `drive_authorized = false`.

That gives us a useful experimental branch without quietly turning software simulation into a hardware claim.

Phase-2 acceptance adds:

```bash
make phase2-check
make host-test
make host-sanitize
make host-analyze
make debug
make release
```

The physical MAX3535/UART path remains disabled until the board routing and motor-controller contract can be validated on hardware.

## Phase 3 on `experimental`

Phase 3 turns the ADC-side joystick information into a real, testable requested-command model while preserving every physical inhibition from the earlier phases.

```text
raw ADC counts
    ↓
validated calibration
    ↓
asymmetric signed Q15 normalization
    ↓
deadband
    ↓
integer response shaping
    ↓
maximum-speed limit
    ↓
AppRequestedDriveCommand
    ↓
Phase-2 authorization model
    ↓
physical command remains zero
```

The first bench reference is the CHC-104B-M2. Phase 3 stores a reference-profile identifier but does not hard-code pretend measurements for a stick we have not characterized yet.

Persistent configuration is modeled as two fixed 64-byte records with generation numbers, explicit little-endian fields, semantic validation, and CRC32/IEEE. The host tests simulate a torn/corrupted newer write and prove that the older complete record remains selectable.

The physical flash backend is deferred. Flash timing and brownout behavior are hardware facts, so hardware gets to answer those questions.

Phase-3 acceptance adds:

```bash
make phase3-check
make host-test
make host-sanitize
make host-analyze
make debug
make release
```

`configuration_valid` in the live Safety Control task remains false and `APP_RS485_PHYSICAL_LINK_ENABLE` remains zero. Experimental means we are allowed to get ahead on software. It does not mean the hardware facts stop mattering.

## Phase 4 on `experimental`

The application now runs the real software pipeline from validated configuration and joystick processing through safety observation and command authorization. Host tests are allowed to inject valid configuration/HMI state and prove non-zero requested motion. The target build still cannot turn that into physical motion.

The HMI task debounces and publishes raw button/rotary state, but final drive-enable semantics remain invalid until hardware testing. The target configuration storage backend likewise reports unavailable until STM32 flash behavior is validated.

## Phase 5 on `experimental`

Phase 5 is the whole-system software abuse pass.  A bounded deterministic simulator runs the real application modules together and injects faults at their boundaries rather than testing each module in isolation.

The safety state may reach logical `DRIVE_AUTHORIZED` in host simulation, but that state is not physical permission.  `CommandAuthorization_Evaluate()` and the disabled RS-485 transport remain the independent physical lock.  Target configuration storage and final HMI enable semantics remain unqualified until hardware validation.

The target linker also has an explicit RX FLASH / RW RAM program-header contract, checked with `readelf` so writable+executable LOAD segments are a build failure.

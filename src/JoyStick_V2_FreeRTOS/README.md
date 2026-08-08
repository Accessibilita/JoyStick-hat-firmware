# JoyStick Interface Firmware — Phase 1 Safety Foundation

This STM32F446VET6 project is the first non-drive-capable firmware phase for the Accessibilita
joystick interface board.

Phase 1 exists to make board bring-up, JTAG/SWD stepping, ADC/DMA verification, RTOS timing, and
fault-injection work possible without creating a path that can command wheelchair motors.

## Installation layouts

This Phase-1 project is distributed in two ZIP layouts: a repository overlay
and a direct STM32CubeIDE-workspace project. See
`docs/OVERLAY_INSTALLATION.md` before extraction.


## Safety lock

The build is intentionally incapable of authorizing movement:

- the current PCB's RS-485/UART direction mismatch is not worked around in software;
- UART5 is not initialized;
- the MAX3535 driver-enable output is held low;
- every published drive command is zero and `drive_authorized = false`;
- the compile-time invariant tool fails if those guards are removed;
- configuration validity remains false until a real calibration/configuration subsystem exists.

Do not connect this Phase 1 firmware to powered traction hardware.

## Project layout

```text
App/            Hand-owned RTOS, safety-state, diagnostics, and mailbox code
Core/           STM32 startup/peripheral initialization and interrupt integration
Platform/       Board-level GPIO ownership wrappers
Drivers/        CMSIS and STM32F4 HAL after dependency bootstrap
Middlewares/    FreeRTOS after dependency bootstrap
docs/           Architecture, pin map, IDE workflow, blockers, and coding standard
tests/host/     Portable state-machine and diagnostic tests
tools/          Offline dependency bootstrap and fail-closed invariant checks
```

This retains the conventional STM32CubeIDE root files and directories: `.project`, `.cproject`,
`.settings/`, `.ioc`, `Core/`, `Drivers/`, `Middlewares/`, and the linker script. `App/` and
`Platform/` keep hand-owned safety code outside generated peripheral files.

## First setup

Install STM32CubeF4 V1.28.1 through STM32CubeIDE, then run:

```bash
tools/ide_preflight.sh
```

Or perform the steps separately:

```bash
tools/bootstrap_cube_dependencies.sh
make host-test
make debug
```

The bootstrap script copies the installed package into `Drivers/` and `Middlewares/`. No network
download occurs during the project build.

## STM32CubeIDE

Import with:

```text
File → Import → General → Existing Projects into Workspace
```

The included Debug and Release configurations call the project-owned GNU Makefile, so the IDE
and terminal compile the same sources with the same linker script.

For the debugger, select:

```text
build/Debug/JoyStick_V2_FreeRTOS.elf
```

Watch this symbol in the Expressions view:

```text
g_app_phase1_debug_snapshot
```

It exposes the current ADC values, input faults, safety state, task-health result, link state,
and the published inhibited command. See `docs/STM32CUBEIDE_WORKFLOW.md` for breakpoints and
bring-up checks.

## Build targets

```bash
make host-test       # Portable tests plus Phase-1 invariant checks
make phase1-check    # Fail-closed source/project checks only
make debug           # ARM Debug ELF, HEX, BIN, and MAP
make release         # ARM Release ELF, HEX, BIN, and MAP
make size            # Report target image size
make flash           # Flash Debug ELF using STM32CubeProgrammer CLI
make clean
```

The Makefile searches the normal STM32Cube repository location and common Linux CubeIDE
installations for the bundled GNU Arm compiler. Both can be overridden:

```bash
make debug \
  STM32CUBE_F4_PATH="$HOME/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1" \
  TOOLCHAIN_PREFIX="/path/to/bin/arm-none-eabi-"
```

## Implemented Phase 1 path

```text
TIM2 1 kHz trigger
    → ADC1 PA0/PA1 scan
    → circular DMA
    → half/full ISR notification every 5 ms
    → Safety Control Task
       → bounded sample reduction
       → range/freshness/PG/overrun diagnostics
       → safety-state transition
       → zero, unauthorized command publication
       → mandatory-task progress check
       → conditional IWDG refresh
```

All application tasks and queues are statically allocated. No FreeRTOS timer-daemon task and no
application heap are used.

## Verification status

Completed in the preparation environment:

- Phase-1 invariant checks;
- strict host compilation of portable modules;
- state-machine and diagnostic tests;
- AddressSanitizer and UndefinedBehaviorSanitizer run;
- XML parsing of Eclipse project metadata.

Not completed in that environment:

- ARM target compilation/linking;
- STM32CubeIDE import validation;
- flashing or execution on the physical board.

Those target checks require your installed STM32CubeF4 package, GNU Arm toolchain, IDE, probe,
and board. The project is structured so `make debug` and the IDE build invoke the same build.

## Read before hardware testing

- `docs/STM32CUBEIDE_WORKFLOW.md`
- `docs/HARDWARE_PINMAP.md`
- `docs/HARDWARE_BLOCKERS.md`
- `docs/PHASE1_ARCHITECTURE.md`
- `docs/CODING_STANDARD.md`
- `docs/CUBEMX_REGENERATION.md`
- `docs/IMPLEMENTATION_STATUS.md`

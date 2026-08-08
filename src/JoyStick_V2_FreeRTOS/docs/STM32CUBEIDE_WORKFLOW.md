# STM32CubeIDE Build and Debug Workflow

STM32CubeIDE is the editor, indexer, debugger, register viewer, and front end.

The project Makefile is the build authority.

That keeps the terminal build and the IDE build from quietly becoming two different firmware projects.

## Toolchain baseline

Current development uses:

```text
STM32CubeIDE 2.2.0
GNU Tools for STM32 14.3.rel1
arm-none-eabi-gcc 14.3.1
```

The Makefile explicitly prefers the compiler bundled with CubeIDE 2.2.0 when that installation exists.

This avoids the previous failure mode where an older CubeIDE installation under `/opt/st` silently won the wildcard lottery.

## Import

In STM32CubeIDE:

```text
File
→ Import
→ General
→ Existing Projects into Workspace
```

Select the directory containing:

```text
.project
.cproject
JoyStick_V2_FreeRTOS.ioc
```

Do not create another copied project unless you actually want another source tree.

## Build configurations

Debug calls:

```bash
make debug
```

Release calls:

```bash
make release
```

Debug uses development-friendly optimization and symbols.

Release builds the optimized target image.

From a shell, the same acceptance sequence is:

```bash
make phase1-check
make clean
make debug
make release
```

## Debug ELF

Use:

```text
build/Debug/JoyStick_V2_FreeRTOS.elf
```

for the debugger.

## Useful first breakpoints

```text
main
MX_GPIO_Init
MX_ADC1_Init
InputAcquisition_OnDmaHalfCompleteFromIsr
InputAcquisition_CopyCompletedBatch
InputDiagnostics_Evaluate
SafetyState_Step
WatchdogSupervision_RefreshIfHealthy
Error_Handler
```

## Debug snapshot

Add this to the Expressions view:

```text
g_app_phase1_debug_snapshot
```

That snapshot exists specifically so we can inspect the important application state without injecting printf traffic into the safety path.

Useful fields include the raw ADC state, diagnosed input condition, safety state, health result, link state, and current inhibited command.

## First board session

Do not start by connecting the motor-control cable.

Start by proving the boring stuff:

1. reset reaches `main`
2. clocks initialize as expected
3. PC9 / RS485 DE stays low
4. LED blanking outputs start safe
5. TIM2 runs at the expected acquisition rate
6. DMA half/full completion occurs at the expected cadence
7. both joystick ADC channels move in the expected direction
8. the safety snapshot advances coherently
9. the system detects intentionally injected input faults
10. the watchdog resets the target when mandatory progress actually stops

Only after that foundation is measured on hardware does the communications link deserve to become interesting.

## Debug watchdog behavior

Debug builds freeze IWDG while halted by the debugger.

That is there so a breakpoint does not look like a firmware hang.

Release builds retain normal watchdog behavior.

## CubeMX warning

Do not press Generate Code over the accepted source tree and assume the result is authoritative.

The `.ioc` is useful, but the reviewed KiCad design exposed real conflicts with the old generated pin configuration.

See `CUBEMX_REGENERATION.md`.

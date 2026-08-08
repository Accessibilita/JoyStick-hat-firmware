# Phase 1 implementation status

## Implemented

- Standard STM32CubeIDE project directories and Eclipse metadata.
- Project-owned GNU Make build using the same source tree as the IDE.
- Static FreeRTOS task and queue creation; dynamic allocation disabled.
- Four-task topology: Safety Control, blocked Link, HMI-safe-state, Diagnostics.
- 96 MHz HSE/PLL clock configuration.
- TIM2-triggered 1 kHz ADC1 scan of PA0/PA1.
- Circular DMA with five samples per axis per safety batch.
- Completed-half copying with sequence and race detection.
- Input age, range, DMA-overrun, and regulator-PG diagnostics.
- Safety state machine and neutral qualification logic.
- Length-one overwrite mailboxes for commands and link status.
- Neutral, explicitly unauthorized command publication only.
- IWDG refresh conditioned on mandatory task progress and no latched fault.
- IWDG debug freeze before watchdog startup for JTAG/SWD stepping.
- LED outputs forced blank and RS-485 output forced disabled.
- Debugger snapshot symbol `g_app_phase1_debug_snapshot`.
- Host tests and compile-time/project invariant checks.

## Deliberately not implemented

- Motor command transmission.
- UART/RS-485 protocol.
- Runtime configuration or calibration storage.
- Joystick response mapping or speed profiles.
- Button debounce and interpreted HMI actions.
- LED driver protocol.
- USB/service interface.
- Any transition to a drive-capable production state.

## Build verification status

The portable modules are compiled with strict warnings and exercised by host tests, including
AddressSanitizer and UndefinedBehaviorSanitizer in the development environment used to prepare
this package.

A complete ARM target link was not run in that environment because it did not contain
STM32CubeF4 V1.28.1 or `arm-none-eabi-gcc`. The package contains an offline dependency bootstrap
script and uses only standard STM32CubeF4 paths. The first local acceptance gate is therefore a
Debug build in STM32CubeIDE and `make debug` using the same installed toolchain.

# Firmware Dependency Baseline

This project intentionally vendors the STM32 firmware dependencies required to
build the JoyStick V2 firmware reproducibly.

## STM32CubeF4

Original project firmware baseline:

- STM32CubeF4: V1.28.1
- Release commit:
  `83778d5c95cb01695c7facbf095db3ab445532ea`

Official upstream:

`STMicroelectronics/STM32CubeF4`

## CMSIS Device F4

The STM32F4 CMSIS device package is the exact Git submodule revision referenced
by STM32CubeF4 V1.28.1:

`5f41fb29d22773896c780052bf61e47fc924d524`

Official upstream:

`STMicroelectronics/cmsis_device_f4`

## STM32F4 HAL Driver

The HAL driver is the exact Git submodule revision referenced by STM32CubeF4
V1.28.1:

`c2e1406d7ea4b73aa42b98ddeb75a8670b1a5a16`

Official upstream:

`STMicroelectronics/stm32f4xx_hal_driver`

## FreeRTOS

FreeRTOS is taken directly from:

STM32CubeF4 V1.28.1

Unlike newer STM32CubeF4 repository layouts, the FreeRTOS middleware used by
V1.28.1 is tracked as part of the parent CubeF4 package rather than through the
later standalone stm32-mw-freertos submodule arrangement.

## Development Toolchain

Phase 1 is currently validated using:

- STM32CubeIDE 2.2.0
- GNU Tools for STM32 14.3.rel1
- arm-none-eabi-gcc 14.3.1

The IDE/toolchain version and STM32CubeF4 firmware-package version are treated as
separate dependencies. STM32CubeF4 V1.28.1 is retained because it is the
historical firmware baseline used by the original project, while development has
moved forward to STM32CubeIDE 2.2.0.

## Phase 1 Build Validation

The following have been successfully built with this dependency set:

- Phase-1 invariant check
- Debug firmware
- Release firmware
- ELF output
- Intel HEX output
- raw BIN output

Hardware execution has not yet been validated because target hardware is not
currently available.

Phase 1 remains intentionally unable to authorize physical motion.

# Firmware Dependency Baseline

The firmware dependencies are vendored deliberately.

A compiler upgrade should not also mean "whatever HAL happened to be current that week."

The original project referenced STM32CubeF4 V1.28.1, so that is the platform baseline retained here.

## STM32CubeF4

```text
STM32CubeF4 V1.28.1
83778d5c95cb01695c7facbf095db3ab445532ea
```

That exact ST release is the parent package used to reconstruct the dependency tree.

## CMSIS Device F4

Exact submodule revision recorded by STM32CubeF4 V1.28.1:

```text
5f41fb29d22773896c780052bf61e47fc924d524
```

Upstream:

```text
STMicroelectronics/cmsis_device_f4
```

## STM32F4 HAL

Exact submodule revision recorded by STM32CubeF4 V1.28.1:

```text
c2e1406d7ea4b73aa42b98ddeb75a8670b1a5a16
```

Upstream:

```text
STMicroelectronics/stm32f4xx_hal_driver
```

## CMSIS Core

The required CMSIS Core headers are taken from the STM32CubeF4 V1.28.1 package.

Only the portions actually required by this firmware are vendored. The full CMSIS DSP/NN/example universe is not useful just because it happened to be in the upstream archive.

## FreeRTOS

FreeRTOS comes directly from the STM32CubeF4 V1.28.1 package.

That release predates the newer CubeF4 layout where this middleware is brought in through the later standalone `stm32-mw-freertos` submodule arrangement.

Keeping the historical package layout matters if we are claiming to reproduce the original firmware baseline.

## Development toolchain

Current builds are validated with:

```text
STM32CubeIDE 2.2.0
GNU Tools for STM32 14.3.rel1
arm-none-eabi-gcc 14.3.1
```

This is intentionally newer than the firmware package.

The IDE/toolchain and the target firmware libraries are separate dependencies.

That gives us the useful combination:

```text
modern compiler + debugger
        ↓
controlled firmware source baseline
        ↓
STM32F446 target
```

instead of silently changing all three layers at once.

## Validation

With this dependency set:

```text
make phase1-check    PASS
make debug           PASS
make release         PASS
```

Both target configurations generate ELF, HEX, and BIN images.

Hardware execution has not yet been validated.

Phase 1 remains incapable of authorizing drive motion.

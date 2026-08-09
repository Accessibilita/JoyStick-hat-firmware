#!/usr/bin/env bash
# SPDX-License-Identifier: MPL-2.0
#
# Accessibilita JoyStick Interface Firmware
# Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
# informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REQUESTED_VERSION="STM32Cube_FW_F4_V1.28.1"

find_cube_root() {
    local candidates=(
        "${STM32CUBE_F4_PATH:-}"
        "$HOME/STM32Cube/Repository/$REQUESTED_VERSION"
        "$HOME/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1"
    )

    local candidate
    for candidate in "${candidates[@]}"; do
        if [[ -n "$candidate" && \
              -f "$candidate/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c" && \
              -f "$candidate/Middlewares/Third_Party/FreeRTOS/Source/tasks.c" && \
              -f "$candidate/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f446xx.s" ]]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    return 1
}

CUBE_ROOT="$(find_cube_root || true)"
if [[ -z "$CUBE_ROOT" ]]; then
    echo "Could not locate STM32CubeF4 V1.28.1." >&2
    echo "Install it through STM32CubeIDE or set STM32CUBE_F4_PATH." >&2
    exit 1
fi

echo "Using STM32CubeF4 package: $CUBE_ROOT"

mkdir -p "$PROJECT_ROOT/Drivers/CMSIS/Device/ST/STM32F4xx"
mkdir -p "$PROJECT_ROOT/Drivers/STM32F4xx_HAL_Driver"
mkdir -p "$PROJECT_ROOT/Middlewares/Third_Party/FreeRTOS"

rm -rf "$PROJECT_ROOT/Drivers/CMSIS/Include"
rm -rf "$PROJECT_ROOT/Drivers/CMSIS/Device/ST/STM32F4xx/Include"
rm -rf "$PROJECT_ROOT/Drivers/CMSIS/Device/ST/STM32F4xx/Source"
rm -rf "$PROJECT_ROOT/Drivers/STM32F4xx_HAL_Driver/Inc"
rm -rf "$PROJECT_ROOT/Drivers/STM32F4xx_HAL_Driver/Src"
rm -rf "$PROJECT_ROOT/Middlewares/Third_Party/FreeRTOS/Source"

cp -a "$CUBE_ROOT/Drivers/CMSIS/Include" \
      "$PROJECT_ROOT/Drivers/CMSIS/"
cp -a "$CUBE_ROOT/Drivers/CMSIS/Device/ST/STM32F4xx/Include" \
      "$PROJECT_ROOT/Drivers/CMSIS/Device/ST/STM32F4xx/"
cp -a "$CUBE_ROOT/Drivers/CMSIS/Device/ST/STM32F4xx/Source" \
      "$PROJECT_ROOT/Drivers/CMSIS/Device/ST/STM32F4xx/"
cp -a "$CUBE_ROOT/Drivers/STM32F4xx_HAL_Driver/Inc" \
      "$PROJECT_ROOT/Drivers/STM32F4xx_HAL_Driver/"
cp -a "$CUBE_ROOT/Drivers/STM32F4xx_HAL_Driver/Src" \
      "$PROJECT_ROOT/Drivers/STM32F4xx_HAL_Driver/"
cp -a "$CUBE_ROOT/Middlewares/Third_Party/FreeRTOS/Source" \
      "$PROJECT_ROOT/Middlewares/Third_Party/FreeRTOS/"

cat > "$PROJECT_ROOT/.cube-dependency-manifest" <<MANIFEST
package=$REQUESTED_VERSION
source=$CUBE_ROOT
bootstrapped_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)
MANIFEST

echo "Dependencies copied into standard CubeIDE project folders."
echo "Next: make host-test && make debug"

#!/usr/bin/env bash
# SPDX-License-Identifier: MPL-2.0
#
# Accessibilita JoyStick Interface Firmware
# Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
# informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.

# Remove obsolete files left behind when the Phase-1 project is extracted over
# the historical STM32CubeIDE project. The Phase-1 Makefile already ignores
# these files, but deleting them keeps CubeIDE's Project Explorer and indexer
# aligned with the code that is actually compiled.
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

if [[ ! -f .project || ! -f Makefile || ! -f JoyStick_V2_FreeRTOS.ioc ]]; then
    echo "Refusing cleanup: $PROJECT_ROOT does not look like JoyStick_V2_FreeRTOS." >&2
    exit 1
fi

legacy_paths=(
    Debug
    Release
    .mxproject
    Core/Startup
    Core/Inc/fmpi2c.h
    Core/Inc/usart.h
    Core/Src/fmpi2c.c
    Core/Src/freertos.c
    Core/Src/stm32f4xx_hal_timebase_tim.c
    Core/Src/syscalls.c
    Core/Src/sysmem.c
    Core/Src/system_stm32f4xx.c
    Core/Src/usart.c
)

removed=0
for path in "${legacy_paths[@]}"; do
    if [[ -e "$path" || -L "$path" ]]; then
        rm -rf -- "$path"
        printf 'Removed legacy path: %s\n' "$path"
        removed=$((removed + 1))
    fi
done

if [[ $removed -eq 0 ]]; then
    echo "No legacy project files required removal."
fi

echo "Running Phase-1 non-drive invariant checks..."
python3 tools/check_phase1_invariants.py

echo
printf '%s\n' \
  'Overlay cleanup complete.' \
  'In STM32CubeIDE: right-click the project -> Refresh, then Project -> Clean.'

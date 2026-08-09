#!/usr/bin/env bash
# SPDX-License-Identifier: MPL-2.0
#
# Accessibilita JoyStick Interface Firmware
# Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
# informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

./tools/bootstrap_cube_dependencies.sh
python3 ./tools/check_phase1_invariants.py
make host-test
make check-tools

echo
echo "Phase 1 preflight passed. Import the project into STM32CubeIDE and build Debug."

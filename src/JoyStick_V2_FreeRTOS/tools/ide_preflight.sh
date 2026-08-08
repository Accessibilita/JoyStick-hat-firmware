#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

./tools/bootstrap_cube_dependencies.sh
python3 ./tools/check_phase1_invariants.py
make host-test
make check-tools

echo
echo "Phase 1 preflight passed. Import the project into STM32CubeIDE and build Debug."

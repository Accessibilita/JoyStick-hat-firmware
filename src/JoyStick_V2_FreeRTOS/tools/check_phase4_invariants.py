#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
#
# Accessibilita JoyStick Interface Firmware
# Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
# informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
"""Fail closed on Phase-4 runtime-integration safety and source-quality invariants."""

from pathlib import Path
import re
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
errors: list[str] = []


def read(relative_path: str) -> str:
    return (root / relative_path).read_text(encoding="utf-8")


def require(relative_path: str, token: str, reason: str) -> None:
    try:
        contents = read(relative_path)
    except OSError as exc:
        errors.append(f"{relative_path}: unable to read file: {exc}")
        return
    if token not in contents:
        errors.append(f"{relative_path}: {reason}")


phase3 = subprocess.run(
    [sys.executable, str(root / "tools/check_phase3_invariants.py")],
    cwd=root,
    check=False,
)
if phase3.returncode != 0:
    errors.append("Phase-3 invariants no longer pass")

require("App/Inc/app_build_config.h", "#define APP_RS485_PHYSICAL_LINK_ENABLE          (0U)", "physical RS-485 must remain disabled")
require("App/Src/safety_control_task.c", "RuntimeControl_Step", "Safety Control is not using the Phase-4 runtime pipeline")
require("App/Src/safety_control_task.c", "ConfigurationRuntime_Load", "validated configuration loader is not wired into runtime")
require("App/Src/safety_control_task.c", "runtime_output.authorization.physical_command", "Safety Control must publish only the authorization boundary output")
require("App/Src/hmi_task.c", "HmiModel_Step", "HMI task is not using the debounced HMI model")
require("App/Src/hmi_model.c", "state->enable_request = false;", "target HMI must remain unable to request drive until mapping validation")
require("App/Src/hmi_model.c", "state->control_mapping_valid = false;", "target HMI mapping must remain unqualified")
require("Platform/Src/configuration_storage.c", "return false;", "target configuration storage must remain unavailable until flash is validated")
require("App/Src/runtime_control.c", "CommandAuthorization_Evaluate", "runtime pipeline bypasses command authorization")
require("App/Src/runtime_control.c", "RuntimeControl_MinU16", "HMI/mode speed policy must only reduce the stored limit")
require("App/Src/runtime_control.c", "RuntimeControl_DeriveCurrentModeFaults", "operating mode must use current-iteration faults instead of stale startup SafetyState faults")
require("App/Src/operating_mode.c", "OperatingMode_GetSpeedCeilingQ15", "operating-mode speed ceiling is missing")
require("tests/host/test_phase4.c", "physical command remains inhibited", "host test must prove the physical-output lock")
require("tests/host/test_phase4.c", "target HMI semantics remain fail-closed", "host test must prove unvalidated HMI semantics cannot enable drive")

# Phase-5 licensing migration puts the declared MPL-2.0 SPDX identifier on
# Phase-4-owned files while preserving coding-standard provenance.
header_files = [
    "App/Inc/hmi_model.h",
    "App/Src/hmi_model.c",
    "App/Inc/configuration_runtime.h",
    "App/Src/configuration_runtime.c",
    "App/Inc/runtime_control.h",
    "App/Src/runtime_control.c",
    "App/Inc/operating_mode.h",
    "App/Src/operating_mode.c",
    "Platform/Inc/configuration_storage.h",
    "Platform/Src/configuration_storage.c",
    "Platform/Src/newlib_stubs.c",
    "App/Inc/app_rtos.h",
    "App/Src/app_rtos.c",
    "App/Inc/app_debug_snapshot.h",
    "App/Src/app_debug_snapshot.c",
    "App/Src/hmi_task.c",
    "App/Src/safety_control_task.c",
    "tests/host/test_phase4.c",
]
for relative_path in header_files:
    try:
        contents = read(relative_path)
    except OSError as exc:
        errors.append(f"{relative_path}: missing Phase-4 owned file: {exc}")
        continue
    header = "\n".join(contents.splitlines()[:14])
    if "SPDX-License-Identifier: MPL-2.0" not in header:
        errors.append(f"{relative_path}: file header does not state MPL-2.0 SPDX license")
    if "Coding standard:" not in header:
        errors.append(f"{relative_path}: file header does not identify coding standard")
    if "MISRA C:2023" not in header:
        errors.append(f"{relative_path}: file header does not identify MISRA C:2023 influence")

prohibited_patterns = {
    r"\bmalloc\s*\(": "malloc",
    r"\bcalloc\s*\(": "calloc",
    r"\brealloc\s*\(": "realloc",
    r"\bfree\s*\(": "free",
    r"\bgoto\b": "goto",
    r"\bfloat\b": "floating-point type",
    r"\bdouble\b": "floating-point type",
}
for relative_path in header_files:
    if not (root / relative_path).exists():
        continue
    contents = read(relative_path)
    for pattern, label in prohibited_patterns.items():
        if re.search(pattern, contents):
            errors.append(f"{relative_path}: prohibited {label}")

# No UART calls are permitted while the reviewed PCB direction mismatch exists.
for directory in (root / "App", root / "Platform"):
    for file_path in directory.rglob("*.[ch]"):
        if re.search(r"\bHAL_UART_", file_path.read_text(encoding="utf-8")):
            errors.append(f"{file_path.relative_to(root)}: physical UART use remains forbidden")

if errors:
    for error in errors:
        print(f"ERROR: {error}", file=sys.stderr)
    raise SystemExit(1)

print("Phase 4 invariants passed")

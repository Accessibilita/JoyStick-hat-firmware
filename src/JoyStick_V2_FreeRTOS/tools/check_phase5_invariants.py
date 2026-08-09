#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
#
# Accessibilita JoyStick Interface Firmware
# Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
# informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
"""Fail closed on Phase-5 system-simulation, license, and output invariants."""

from __future__ import annotations

from pathlib import Path
import re
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
repo_root = root.parents[1]
errors: list[str] = []


def read(relative_path: str) -> str:
    return (root / relative_path).read_text(encoding="utf-8")


def require(relative_path: str, token: str, reason: str) -> None:
    try:
        contents = read(relative_path)
    except OSError as exc:
        errors.append(f"{relative_path}: unable to read required file: {exc}")
        return
    if token not in contents:
        errors.append(f"{relative_path}: {reason}")


phase4 = subprocess.run(
    [sys.executable, str(root / "tools/check_phase4_invariants.py")],
    cwd=root,
    check=False,
)
if phase4.returncode != 0:
    errors.append("Phase-4 invariants no longer pass")

license_path = repo_root / "LICENSE"
try:
    license_text = license_path.read_text(encoding="utf-8")
except OSError as exc:
    errors.append(f"LICENSE: repository MPL-2.0 text is missing: {exc}")
else:
    if not license_text.startswith("Mozilla Public License Version 2.0"):
        errors.append("LICENSE: expected Mozilla Public License Version 2.0")
    if 'Exhibit B - "Incompatible With Secondary Licenses" Notice' not in license_text:
        errors.append("LICENSE: MPL-2.0 Exhibit B is missing")

require(
    "App/Inc/app_build_config.h",
    "#define APP_RS485_PHYSICAL_LINK_ENABLE          (0U)",
    "physical RS-485 must remain compile-time disabled",
)
require(
    "App/Src/rs485_link_task.c",
    '#error "Phase 1 must not enable the present PCB RS-485 data connection."',
    "blocked PCB transport guard is missing",
)
for token in (
    "command.drive_authorized = false;",
    "command.forward_q15 = 0;",
    "command.turn_q15 = 0;",
):
    require(
        "App/Src/command_authorization.c",
        token,
        "physical command zero/inhibit boundary changed",
    )
require(
    "App/Src/command_authorization.c",
    '#error "Phase 2 software authorization model requires physical drive to remain disabled."',
    "physical-output compile guard is missing",
)

# Phase 5 intentionally permits the logical safety state to reach DRIVE_AUTHORIZED
# so the integrated software model can be exercised without conflating that state
# with physical transport permission.
require(
    "App/Src/safety_state.c",
    "This is logical authorization only.",
    "logical/physical authorization separation is not documented",
)
if "#if APP_RS485_PHYSICAL_LINK_ENABLE" in read("App/Src/safety_state.c"):
    errors.append(
        "App/Src/safety_state.c: logical safety state must not depend on physical transport enable"
    )

require(
    "App/Src/motor_link_state.c",
    "A remote fault invalidates the pre-fault qualification history.",
    "remote-fault requalification rule is missing",
)
require(
    "tests/host/test_phase5.c",
    "PHASE5_CAMPAIGN_STEPS                   (20000U)",
    "long deterministic full-system campaign is missing or reduced",
)
for token, reason in (
    ("logical drive authorization is reachable", "logical authorization system test is missing"),
    ("one post-fault good frame cannot restore link validity", "remote-fault requalification test is missing"),
    ("reboot with displaced stick cannot resume logical drive", "reboot safety test is missing"),
    ("neutral/link timing remains correct across uint32 tick wrap", "tick-wrap system test is missing"),
    ("logical authorization never becomes physical output", "logical/physical separation test is missing"),
):
    require("tests/host/test_phase5.c", token, reason)

require("STM32F446VETX_FLASH.ld", "PHDRS", "explicit ELF program headers are missing")
require("STM32F446VETX_FLASH.ld", "flash PT_LOAD FLAGS(5);", "FLASH segment must be RX")
require("STM32F446VETX_FLASH.ld", "ram   PT_LOAD FLAGS(6);", "RAM segment must be RW")
require("tools/check_elf_segments.py", "writable+executable LOAD segment", "RWX ELF gate is missing")

# Build-control files are project-owned code too.  Keep their license and
# engineering-standard provenance machine-checkable rather than ceremonial.
for build_file in ("Makefile", "tests/host/Makefile", "STM32F446VETX_FLASH.ld"):
    require(build_file, "SPDX-License-Identifier: MPL-2.0", "MPL-2.0 SPDX header is missing")
    require(build_file, "Coding standard:", "coding-standard provenance is missing")
    require(build_file, "MISRA C:2023", "MISRA C:2023 provenance is missing")

# Project-owned source carries MPL-2.0 SPDX and coding-standard provenance.
# Vendor/third-party trees are intentionally excluded so upstream notices remain intact.
owned_roots = [root / "App", root / "Platform", root / "Core", root / "tests/host", root / "tools"]
owned_extensions = {".c", ".h", ".py", ".sh"}
prohibited_patterns = {
    r"\bmalloc\s*\(": "malloc",
    r"\bcalloc\s*\(": "calloc",
    r"\brealloc\s*\(": "realloc",
    r"\bfree\s*\(": "free",
    r"\bgoto\b": "goto",
    r"\bfloat\b": "floating-point type",
    r"\bdouble\b": "floating-point type",
}

for owned_root in owned_roots:
    for file_path in owned_root.rglob("*"):
        if (not file_path.is_file()) or (file_path.suffix not in owned_extensions):
            continue
        contents = file_path.read_text(encoding="utf-8")
        header = "\n".join(contents.splitlines()[:14])
        relative = file_path.relative_to(root)
        if "SPDX-License-Identifier: MPL-2.0" not in header:
            errors.append(f"{relative}: MPL-2.0 SPDX header is missing")
        if "Coding standard:" not in header:
            errors.append(f"{relative}: coding-standard provenance is missing")
        if "MISRA C:2023" not in header:
            errors.append(f"{relative}: MISRA C:2023 provenance is missing")

        # Python/shell support tools are not target firmware and may legitimately
        # use language features such as dynamic objects. C/H files retain the
        # strict embedded subset here.
        if file_path.suffix in {".c", ".h"}:
            for pattern, label in prohibited_patterns.items():
                if re.search(pattern, contents):
                    errors.append(f"{relative}: prohibited {label}")

# Current hardware blocker remains absolute: no application/platform UART path.
for directory in (root / "App", root / "Platform"):
    for file_path in directory.rglob("*.[ch]"):
        if re.search(r"\bHAL_UART_", file_path.read_text(encoding="utf-8")):
            errors.append(f"{file_path.relative_to(root)}: physical UART use remains forbidden")

# Target persistence and target HMI semantics are still unqualified.
require(
    "Platform/Src/configuration_storage.c",
    "return false;",
    "target configuration storage must remain disabled until hardware validation",
)
require(
    "App/Src/hmi_model.c",
    "state->control_mapping_valid = false;",
    "target HMI mapping must remain hardware-unqualified",
)

if errors:
    for error in errors:
        print(f"ERROR: {error}", file=sys.stderr)
    raise SystemExit(1)

print("Phase 5 invariants passed")

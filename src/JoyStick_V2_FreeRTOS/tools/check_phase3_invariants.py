#!/usr/bin/env python3
"""Fail closed when a Phase-3 software-only safety invariant is removed."""

from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
errors: list[str] = []


def text(relative_path: str) -> str:
    return (root / relative_path).read_text(encoding="utf-8")


def require(relative_path: str, token: str, reason: str) -> None:
    try:
        contents = text(relative_path)
    except OSError as exc:
        errors.append(f"{relative_path}: cannot read required file: {exc}")
        return
    if token not in contents:
        errors.append(f"{relative_path}: {reason}")


# Earlier phase invariants remain authority. Phase 3 is additive.
phase2_checker = root / "tools/check_phase2_invariants.py"
if not phase2_checker.exists():
    errors.append("tools/check_phase2_invariants.py: Phase-2 checker is missing")

require(
    "App/Inc/app_build_config.h",
    "#define APP_RS485_PHYSICAL_LINK_ENABLE          (0U)",
    "physical RS-485 link must remain disabled",
)
require(
    "App/Src/safety_control_task.c",
    "observation.configuration_valid = false;",
    "runtime configuration must remain unqualified until hardware-backed integration",
)
require(
    "App/Src/command_authorization.c",
    "command.drive_authorized = false;",
    "physical command must remain explicitly unauthorized",
)
require(
    "App/Src/command_authorization.c",
    "command.forward_q15 = 0;",
    "physical forward demand must remain zero",
)
require(
    "App/Src/command_authorization.c",
    "command.turn_q15 = 0;",
    "physical turn demand must remain zero",
)

require(
    "App/Inc/joystick_configuration.h",
    "#define JOYSTICK_CONFIG_RECORD_SIZE                    (64U)",
    "persistent configuration record size changed without review",
)
require(
    "App/Inc/joystick_configuration.h",
    "JOYSTICK_CONFIG_REFERENCE_CHC_104B_M2",
    "CHC-104B-M2 reference profile marker is missing",
)
require(
    "App/Src/joystick_configuration.c",
    "0xEDB88320UL",
    "CRC32 implementation/polynomial changed without review",
)
require(
    "App/Src/joystick_processing.c",
    "JoystickProcessing_BuildRequestedDriveCommand",
    "requested-command pipeline is missing",
)
require(
    "App/Src/joystick_processing.c",
    "request->enable_request = false;",
    "failed processing must default the request to inhibited",
)
require(
    "Core/Inc/stm32f4xx_hal_conf.h",
    "#define USE_SPI_CRC                              0U",
    "historical HAL SPI CRC setting must be explicit under -Wundef",
)

phase3_owned = [
    root / "App/Inc/joystick_configuration.h",
    root / "App/Inc/joystick_calibration.h",
    root / "App/Inc/joystick_processing.h",
    root / "App/Src/joystick_configuration.c",
    root / "App/Src/joystick_calibration.c",
    root / "App/Src/joystick_processing.c",
]

prohibited_patterns = {
    r"\bmalloc\s*\(": "malloc",
    r"\bcalloc\s*\(": "calloc",
    r"\brealloc\s*\(": "realloc",
    r"\bfree\s*\(": "free",
    r"\bgoto\b": "goto",
    r"\bfloat\b": "floating-point type",
    r"\bdouble\b": "floating-point type",
    r"HAL_FLASH_Program\s*\(": "direct flash programming in portable Phase-3 model",
}

for file_path in phase3_owned:
    if not file_path.exists():
        errors.append(f"{file_path.relative_to(root)}: required Phase-3 file is missing")
        continue
    contents = file_path.read_text(encoding="utf-8")
    for pattern, label in prohibited_patterns.items():
        if re.search(pattern, contents):
            errors.append(f"{file_path.relative_to(root)}: prohibited {label}")

require(
    "tests/host/test_phase3.c",
    "0xCBF43926",
    "CRC32 golden vector is missing",
)
require(
    "tests/host/test_phase3.c",
    "Test_RedundantSlotSelectionSurvivesTornUpdate",
    "transactional two-slot recovery test is missing",
)
require(
    "tests/host/test_phase3.c",
    "Test_BoundedProcessingAcrossAdcDomain",
    "full ADC-domain randomized boundedness test is missing",
)
require(
    "tests/host/test_phase3.c",
    "Test_RandomRecordsNeverEscapeValidation",
    "malformed configuration record campaign is missing",
)

if errors:
    for error in errors:
        print(f"ERROR: {error}", file=sys.stderr)
    raise SystemExit(1)

print("Phase 3 invariants passed")

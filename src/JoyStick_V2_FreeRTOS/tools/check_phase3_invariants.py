#!/usr/bin/env python3
"""Preserve Phase-3 calibration/configuration invariants through later phases."""

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


require("App/Inc/app_build_config.h", "#define APP_RS485_PHYSICAL_LINK_ENABLE          (0U)", "physical RS-485 link must remain disabled")
for token in ("command.drive_authorized = false;", "command.forward_q15 = 0;", "command.turn_q15 = 0;"):
    require("App/Src/command_authorization.c", token, "physical command inhibition changed")
require("App/Inc/joystick_configuration.h", "#define JOYSTICK_CONFIG_RECORD_SIZE                    (64U)", "persistent record size changed")
require("App/Inc/joystick_configuration.h", "JOYSTICK_CONFIG_REFERENCE_CHC_104B_M2", "CHC-104B-M2 reference marker is missing")
require("App/Src/joystick_configuration.c", "0xEDB88320UL", "CRC32 polynomial changed")
require("App/Src/joystick_processing.c", "JoystickProcessing_BuildRequestedDriveCommand", "requested-command pipeline is missing")
require("App/Src/joystick_processing.c", "request->enable_request = false;", "failed processing must default to inhibited")
require("Core/Inc/stm32f4xx_hal_conf.h", "#define USE_SPI_CRC                              0U", "historical HAL SPI CRC setting must remain explicit")

# Phase 3 originally forced configuration_valid=false in Safety Control.  Phase
# 4 is allowed to replace that placeholder only with the reviewed runtime loader.
safety_source = text("App/Src/safety_control_task.c")
if ("configuration_valid = false;" not in safety_source) and ("ConfigurationRuntime_Load" not in safety_source):
    errors.append("App/Src/safety_control_task.c: configuration authority changed without runtime validation")

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

for token, reason in (
    ("0xCBF43926", "CRC32 golden vector is missing"),
    ("Test_RedundantSlotSelectionSurvivesTornUpdate", "two-slot recovery test is missing"),
    ("Test_BoundedProcessingAcrossAdcDomain", "ADC-domain boundedness test is missing"),
    ("Test_RandomRecordsNeverEscapeValidation", "malformed-record campaign is missing"),
):
    require("tests/host/test_phase3.c", token, reason)

if errors:
    for error in errors:
        print(f"ERROR: {error}", file=sys.stderr)
    raise SystemExit(1)

print("Phase 3 invariants passed")

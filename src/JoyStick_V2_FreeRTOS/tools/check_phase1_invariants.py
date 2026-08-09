#!/usr/bin/env python3
"""Preserve the original non-drive invariants as later experimental phases evolve."""

from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET

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


require(
    "App/Inc/app_build_config.h",
    "#define APP_RS485_PHYSICAL_LINK_ENABLE          (0U)",
    "RS-485 physical link must remain compile-time disabled",
)
require(
    "App/Src/rs485_link_task.c",
    '#error "Phase 1 must not enable the present PCB RS-485 data connection."',
    "missing compile-time guard against enabling the blocked physical link",
)

# Later phases moved the final physical-zero construction out of Safety Control
# and into command_authorization.c.  Preserve the behavior, not an obsolete file
# location from the Phase-1 implementation.
for token in (
    "command.drive_authorized = false;",
    "command.forward_q15 = 0;",
    "command.turn_q15 = 0;",
):
    require(
        "App/Src/command_authorization.c",
        token,
        "original Phase-1 non-drive output invariant is missing",
    )

require(
    "Core/Inc/FreeRTOSConfig.h",
    "#define configSUPPORT_DYNAMIC_ALLOCATION        0",
    "FreeRTOS dynamic allocation must remain disabled",
)
require(
    "Core/Inc/FreeRTOSConfig.h",
    "#define configSUPPORT_STATIC_ALLOCATION         1",
    "FreeRTOS static allocation must remain enabled",
)
require(
    "Core/Inc/FreeRTOSConfig.h",
    "#define configUSE_TIMERS                        0",
    "the shared timer-daemon task remains intentionally absent",
)

main_header = text("Core/Inc/main.h")
for token in (
    "RS485_nRE_Pin                  GPIO_PIN_8",
    "RS485_DE_Pin                   GPIO_PIN_9",
    "RS485_DI_SAFE_Pin              GPIO_PIN_7",
    "RS485_RO_SENSE_Pin             GPIO_PIN_8",
    "JOYSTICK_Y_Pin                 GPIO_PIN_0",
    "JOYSTICK_X_Pin                 GPIO_PIN_1",
):
    if token not in main_header:
        errors.append(f"Core/Inc/main.h: changed reviewed pin definition: {token}")

for prohibited in ("Core/Src/usart.c", "Core/Inc/usart.h"):
    if (root / prohibited).exists():
        errors.append(f"{prohibited}: UART must remain absent until hardware is revised")

hand_owned = list((root / "App").rglob("*.[ch]"))
hand_owned += list((root / "Platform").rglob("*.[ch]"))
hand_owned += list((root / "Core").rglob("*.[ch]"))
prohibited_patterns = {
    r"\bmalloc\s*\(": "malloc",
    r"\bcalloc\s*\(": "calloc",
    r"\brealloc\s*\(": "realloc",
    r"\bfree\s*\(": "free",
    r"\bpvPortMalloc\s*\(": "pvPortMalloc",
    r"\bvPortFree\s*\(": "vPortFree",
    r"\bxTaskCreate\s*\(": "dynamic xTaskCreate",
    r"\bxQueueCreate\s*\(": "dynamic xQueueCreate",
}
for file_path in hand_owned:
    contents = file_path.read_text(encoding="utf-8")
    for pattern, label in prohibited_patterns.items():
        if re.search(pattern, contents):
            errors.append(f"{file_path.relative_to(root)}: prohibited {label}")

for relative_path in (".project", ".cproject", ".settings/language.settings.xml"):
    try:
        ET.parse(root / relative_path)
    except (OSError, ET.ParseError) as exc:
        errors.append(f"{relative_path}: invalid IDE metadata: {exc}")

if errors:
    for error in errors:
        print(f"ERROR: {error}", file=sys.stderr)
    raise SystemExit(1)

print("Phase 1 invariants passed")

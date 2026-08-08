#!/usr/bin/env python3
"""Fail closed when a Phase-1 non-drive invariant is accidentally removed."""

from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[1]
errors: list[str] = []


def text(relative_path: str) -> str:
    return (root / relative_path).read_text(encoding="utf-8")


def require(relative_path: str, token: str, reason: str) -> None:
    if token not in text(relative_path):
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
require(
    "App/Src/safety_control_task.c",
    "command.drive_authorized = false;",
    "published commands must remain explicitly unauthorized",
)
for token in ("command.forward_q15 = 0;", "command.turn_q15 = 0;"):
    require(
        "App/Src/safety_control_task.c",
        token,
        "all Phase-1 motion demand fields must be zero",
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
    "the shared timer-daemon task is intentionally absent in Phase 1",
)

main_header = text("Core/Inc/main.h")
required_pin_tokens = (
    "RS485_nRE_Pin                  GPIO_PIN_8",
    "RS485_DE_Pin                   GPIO_PIN_9",
    "RS485_DI_SAFE_Pin              GPIO_PIN_7",
    "RS485_RO_SENSE_Pin             GPIO_PIN_8",
    "JOYSTICK_Y_Pin                 GPIO_PIN_0",
    "JOYSTICK_X_Pin                 GPIO_PIN_1",
)
for token in required_pin_tokens:
    if token not in main_header:
        errors.append(f"Core/Inc/main.h: changed reviewed pin definition: {token}")

# The existing PCB cannot use UART5 because DI and RO land on the opposite MCU
# directions. Phase 1 must therefore contain no USART/UART driver module.
for prohibited in ("Core/Src/usart.c", "Core/Inc/usart.h"):
    if (root / prohibited).exists():
        errors.append(f"{prohibited}: UART must remain absent until hardware is revised")

# Ban ordinary heap and dynamic RTOS constructors in hand-owned code.
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

# Project metadata must remain parseable for STM32CubeIDE import.
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

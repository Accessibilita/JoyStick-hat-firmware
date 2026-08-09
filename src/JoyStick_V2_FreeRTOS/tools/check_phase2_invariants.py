#!/usr/bin/env python3
"""Phase-2 software-only invariants for the experimental branch.

Phase 2 is allowed to model logical authorization and a complete motor-link
protocol, but it is not allowed to create a physical motor-output path.
"""

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


def require_regex(relative_path: str, pattern: str, reason: str) -> None:
    try:
        contents = read(relative_path)
    except OSError as exc:
        errors.append(f"{relative_path}: unable to read file: {exc}")
        return

    if re.search(pattern, contents, flags=re.MULTILINE) is None:
        errors.append(f"{relative_path}: {reason}")


phase1 = subprocess.run(
    [sys.executable, str(root / "tools/check_phase1_invariants.py")],
    cwd=root,
    check=False,
)
if phase1.returncode != 0:
    errors.append("Phase-1 non-drive invariants no longer pass")

require_regex(
    "App/Inc/app_build_config.h",
    r"^#define\s+APP_RS485_PHYSICAL_LINK_ENABLE\s+\(0U\)\s*$",
    "physical RS-485/motor output must remain disabled",
)
require_regex(
    "App/Inc/motor_protocol.h",
    r"^#define\s+MOTOR_PROTOCOL_FRAME_SIZE\s+\(32U\)\s*$",
    "Phase-2 protocol must remain a fixed 32-byte frame",
)
require_regex(
    "App/Inc/motor_protocol.h",
    r"^#define\s+MOTOR_PROTOCOL_VERSION\s+\(1U\)\s*$",
    "protocol version must be explicit",
)
require_regex(
    "App/Inc/motor_protocol.h",
    r"^#define\s+MOTOR_PROTOCOL_ADDRESS_BROADCAST\s+\(0xFFU\)\s*$",
    "broadcast address must remain explicit",
)
require_regex(
    "App/Inc/app_types.h",
    r"^#define\s+APP_FAULT_PROTOCOL_ADDRESS\s+\(\(AppFaultMask\)\(1UL << 17\)\)\s*$",
    "protocol address faults must remain visible in the application fault vocabulary",
)
require(
    "App/Inc/app_types.h",
    "uint32_t link_fault_history;",
    "motor-link diagnostics must expose local protocol/link fault history",
)
require(
    "App/Src/motor_protocol.c",
    "0x1021U",
    "CRC-16/CCITT-FALSE polynomial must remain explicit",
)
require(
    "App/Src/command_authorization.c",
    'command.forward_q15 = 0;',
    "physical command must stay zero even when logical authorization succeeds",
)
require(
    "App/Src/command_authorization.c",
    'command.turn_q15 = 0;',
    "physical turn command must stay zero",
)
require(
    "App/Src/command_authorization.c",
    'command.drive_authorized = false;',
    "Phase-2 physical command must remain unauthorized",
)
require(
    "App/Src/command_authorization.c",
    '#error "Phase 2 software authorization model requires physical drive to remain disabled."',
    "missing compile-time guard around Phase-2 physical inhibition",
)
require(
    "App/Src/motor_link_state.c",
    "status->destination_address != context->local_node_address",
    "link state must reject traffic addressed to the wrong node",
)
require(
    "App/Src/motor_link_state.c",
    "status->source_address != context->remote_node_address",
    "link state must reject traffic from the wrong node",
)
require(
    "App/Src/motor_link_state.c",
    "status->command_session_echo != context->local_command_session_id",
    "responses must be bound to the current command session",
)
require(
    "App/Src/motor_link_state.c",
    "status->acknowledged_sequence != expected_command_sequence",
    "responses must acknowledge the current command sequence",
)
require(
    "tests/host/test_phase2.c",
    "Test_LinkRejectsWrongAddress",
    "host tests must retain wrong-node rejection",
)
require(
    "tests/host/test_phase2.c",
    "Test_MalformedFramesNeverEscapeDecoderContract",
    "host malformed-frame campaign must remain present",
)
require(
    "tests/host/fake_motor_controller.c",
    "FAKE_MOTOR_MODE_CORRUPT_CRC",
    "fake controller must retain fault injection",
)

protocol_source = read("App/Src/motor_protocol.c")
for forbidden, reason in (
    (r"#\s*pragma\s+pack", "packed wire structs are forbidden"),
    (r"__attribute__\s*\(\(packed\)\)", "packed wire structs are forbidden"),
    (r"\bmemcpy\s*\(", "wire frames must be decoded field-by-field"),
):
    if re.search(forbidden, protocol_source):
        errors.append(f"App/Src/motor_protocol.c: {reason}")

# Until the board routing is fixed, owned application/platform code must not
# gain a UART call behind the invariant checker's back.
for directory in (root / "App", root / "Platform"):
    for file_path in directory.rglob("*.[ch]"):
        contents = file_path.read_text(encoding="utf-8")
        if re.search(r"\bHAL_UART_", contents):
            errors.append(
                f"{file_path.relative_to(root)}: physical UART use is forbidden in Phase 2"
            )

if errors:
    for error in errors:
        print(f"ERROR: {error}", file=sys.stderr)
    raise SystemExit(1)

print("Phase 2 invariants passed")

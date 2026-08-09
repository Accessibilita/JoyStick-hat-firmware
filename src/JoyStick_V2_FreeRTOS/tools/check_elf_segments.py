#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
#
# Accessibilita JoyStick Interface Firmware
# Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
# informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
"""Reject writable+executable ELF LOAD segments in a built target image."""

from __future__ import annotations

from pathlib import Path
import subprocess
import sys


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if len(sys.argv) != 3:
        fail("usage: check_elf_segments.py <readelf> <firmware.elf>")

    readelf = sys.argv[1]
    elf = Path(sys.argv[2])
    if not elf.is_file():
        fail(f"ELF not found: {elf}")

    result = subprocess.run(
        [readelf, "-lW", str(elf)],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    if result.returncode != 0:
        fail(f"readelf failed ({result.returncode})\n{result.stdout}")

    load_count = 0
    rwx_lines: list[str] = []
    for raw_line in result.stdout.splitlines():
        line = raw_line.strip()
        if not line.startswith("LOAD"):
            continue
        load_count += 1
        fields = line.split()
        if len(fields) < 8:
            fail(f"unrecognized LOAD line: {raw_line}")

        # GNU readelf prints flags between the MemSiz and Align columns.  RX is
        # typically two fields ("R", "E") while RW is one field ("RW").
        flags = "".join(fields[6:-1])
        if ("W" in flags) and ("E" in flags):
            rwx_lines.append(raw_line)

    if load_count < 2:
        fail(f"expected separate FLASH/RAM LOAD segments, found {load_count}")
    if rwx_lines:
        fail("writable+executable LOAD segment(s) found:\n" + "\n".join(rwx_lines))

    print(f"ELF segment policy passed ({load_count} LOAD segments, no RWX)")


if __name__ == "__main__":
    main()

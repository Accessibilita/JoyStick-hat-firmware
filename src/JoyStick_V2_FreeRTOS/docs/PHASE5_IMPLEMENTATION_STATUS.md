# Phase 5 implementation status

## Implemented

- deterministic full-system host simulator;
- real RuntimeControl / SafetyState / CommandAuthorization integration in the simulation;
- real Motor Protocol V1 encoding/decoding in the simulation;
- real MotorLinkState feedback loop;
- fake motor-controller fault injection;
- logical-vs-physical authorization separation;
- remote-fault link requalification hardening;
- controller restart and reboot scenarios;
- input/configuration/task/HMI-mode fault scenarios;
- 32-bit tick-wrap scenario;
- 20,000-step deterministic mixed-fault campaign;
- MPL-2.0 repository license and SPDX migration for project-owned code;
- explicit RX FLASH / RW RAM ELF program headers;
- Debug/Release RWX segment acceptance checker.

## Software validation required on the development host

The Phase-5 installer does not commit until these all succeed on the actual project tree:

```text
Phase 1 invariants
Phase 2 invariants
Phase 3 invariants
Phase 4 invariants
Phase 5 invariants
host tests
ASan + UBSan
GCC -fanalyzer
Debug target build
Debug ELF segment check
Release target build
Release ELF segment check
```

## Deliberately not implemented as target authority

- physical RS-485 transport;
- target HMI drive-enable mapping;
- target flash configuration writes;
- production CHC-104B-M2 calibration values.

Those remain hardware-validation debt, not software TODOs to paper over.

## Physical-drive invariant

Phase 5 may produce a non-zero `AppRequestedDriveCommand` and may prove `logical_authorized=true` in host simulation.

The `AppAuthorizedDriveCommand` returned across the physical boundary remains:

```text
forward_q15       = 0
turn_q15          = 0
maximum_speed_q15 = 0
drive_authorized  = false
```

That invariant stays until the hardware blockers are resolved and a later phase explicitly changes the physical boundary.

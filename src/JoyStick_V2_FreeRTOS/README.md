# JoyStick V2 Firmware — `experimental`

This is the newest ahead-of-hardware STM32F446 firmware tree for the Accessibilita joystick interface.

Phase 1 through Phase 5 are integrated here. The software model is deliberately mature enough to test logical drive authorization while the physical output path remains independently locked out.

## Current software path

```text
TIM2 / ADC1 / DMA
        ↓
input diagnostics
        ↓
validated configuration
        ↓
joystick calibration + signed Q15 shaping
        ↓
HMI + operating-mode limits
        ↓
AppRequestedDriveCommand
        ↓
SafetyState
        ↓
MotorLinkState
        ↓
logical authorization
        ↓
CommandAuthorization physical boundary
        ↓
ZERO / UNAUTHORIZED physical command
```

That last boundary is intentional. Logical authorization is testable; physical permission is still impossible in the current target build.

## Phase status

- **Phase 1:** acquisition/safety/RTOS/watchdog foundation
- **Phase 2:** 32-byte addressed motor protocol, CRC/session/sequence/link model
- **Phase 3:** CHC-104B-M2 reference calibration model, Q15 shaping, redundant configuration records
- **Phase 4:** live runtime integration, HMI debounce/model, operating modes, configuration authority plumbing
- **Phase 5:** deterministic whole-system simulator, cross-module fault campaign, logical-vs-physical authorization split, ELF segment policy

## Phase-5 acceptance

The milestone passed:

```text
Phase 1–5 invariants
Phase 1–5 host tests
ASan + UBSan
GCC -fanalyzer
20,000-step deterministic system-abuse campaign
STM32 Debug build
STM32 Release build
ELF segment check: 2 LOAD segments, no RWX
```

This is **software validation and target-build evidence**. It is not hardware qualification.

## Still intentionally unqualified

- physical RS-485 transport
- final target configuration-flash backend
- final HMI drive-enable semantics
- real CHC-104B-M2 center/endpoints/noise
- physical motor-controller behavior
- board reset/watchdog/timing measurements
- complete hardware/system safety

## Branch and roadmap

Read repository-level:

- `../../docs/BRANCH_MODEL.md`
- `../../docs/ROADMAP.md`
- `../../docs/VALIDATION_LEVELS.md`

The next major milestone is a hardware-validation campaign, not another speculative feature phase.

## Coding and licensing

Project-owned code follows `docs/CODING_STANDARD.md` and is covered by MPL-2.0. Phase 5 applied broad `SPDX-License-Identifier: MPL-2.0` coverage to project-owned source/build logic. Imported `Drivers/` and `Middlewares/` code keeps its upstream license.

## Detailed engineering record

Start with:

- `docs/PHASE5_ARCHITECTURE.md`
- `docs/PHASE5_FAULT_MATRIX.md`
- `docs/PHASE5_TEST_PLAN.md`
- `docs/PHASE5_IMPLEMENTATION_STATUS.md`
- `docs/MOTOR_PROTOCOL.md`
- `docs/CONFIGURATION_FORMAT.md`
- `docs/CHC104B_M2_REFERENCE.md`
- `docs/HARDWARE_BLOCKERS.md`
- `docs/HARDWARE_PINMAP.md`
- `docs/DEPENDENCIES.md`

A successful simulation is evidence. A successful target build is evidence. Neither one is an oscilloscope trace.

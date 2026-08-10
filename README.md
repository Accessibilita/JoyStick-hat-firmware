# JoyStick Interface Firmware

Firmware for the Accessibilita joystick interface platform, built around the STM32F446VET6.

> **Branch: `experimental` — newest ahead-of-hardware engineering.** This branch currently carries Phase 1 through Phase 5. It is software-validated and target-built at the Phase-5 milestone; it is **not** a hardware-qualified mobility release.

**Raw joystick data is not a motor command.**

Phase 5 now exercises the complete software path far enough to allow logical drive authorization in simulation while an independent physical authorization/transport boundary still forces real output to zero.

## Current milestone

```text
Phase 1  safety foundation                         DONE — software/target evidence
Phase 2  motor protocol + link model                DONE — software-validated
Phase 3  calibration/configuration/shaping          DONE — software-validated
Phase 4  runtime integration + HMI/modes            DONE — software-validated
Phase 5  full-system simulation + fault campaign    DONE — software-validated + target-built

Physical drive                                     LOCKED OUT
Hardware validation                                NOT CLAIMED
```

The Phase-5 host campaign runs the real application modules together and covers controller restart, CRC/stale-ACK/silence faults, configuration loss, task-health failure, power-good/ADC faults, service/calibration modes, reboot with a displaced stick, tick wrap, and a deterministic 20,000-step abuse campaign.

## Branch model

`experimental` is allowed to get ahead of hardware. It is not allowed to get sloppy.

See [`docs/BRANCH_MODEL.md`](docs/BRANCH_MODEL.md) for promotion rules and [`docs/VALIDATION_LEVELS.md`](docs/VALIDATION_LEVELS.md) for the evidence vocabulary.

## Next move

The next major milestone is hardware evidence, not another pile of speculative features. [`docs/ROADMAP.md`](docs/ROADMAP.md) turns the remaining debt into a bench sequence: board startup, real CHC-104B-M2 characterization, HMI mapping, RS-485 resolution, motor-controller integration, flash behavior, and controlled physical-motion validation.

## Engineering standard

Project-owned code follows [`docs/CODING_STANDARD.md`](docs/CODING_STANDARD.md). Phase 5 also audits project-owned source/build files for MPL-2.0/SPDX and coding-standard provenance where appropriate.

## License

Project-owned source is covered by **Mozilla Public License 2.0 (MPL-2.0)**. See [`LICENSE`](LICENSE) and [`docs/LICENSING.md`](docs/LICENSING.md). Imported STM32/CMSIS/FreeRTOS source retains upstream terms.

## Firmware project

The STM32 project lives at:

```text
src/JoyStick_V2_FreeRTOS/
```

Its `docs/` directory contains the detailed architecture, protocol, calibration, phase test plans, fault matrix, hardware blockers, pin map, and dependency record.

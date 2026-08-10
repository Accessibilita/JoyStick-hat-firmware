# JoyStick Interface Firmware

Firmware for the Accessibilita joystick interface platform.

> **Branch: `main` — stable / release-facing history.** This is intentionally not the newest development branch. Hardware-facing development is on `edge`; ahead-of-hardware software work is on `experimental`.

The project is built around one non-negotiable boundary:

**Raw joystick data is not a motor command.**

Acquisition, diagnostics, calibration, requested motion, safety state, communications state, logical authorization, and physical output authority are separate things so each can be reasoned about and tested.

## Where the branches are

| Branch | What a reader should expect |
|---|---|
| `main` | stable historical / release-facing baseline |
| `edge` | Phase-1 safety foundation and hardware-facing bring-up baseline |
| `experimental` | Phase-1 through Phase-5 software architecture, integration, and full-system fault simulation |

Read [`docs/BRANCH_MODEL.md`](docs/BRANCH_MODEL.md) before assuming the newest phase belongs on this branch.

## Current project state

The newest `experimental` milestone has reached **software validation through Phase 5 and successful STM32 target builds**. The complete electromechanical system is **not hardware-validated**, the physical drive path remains inhibited, and several hardware facts remain unresolved.

This `main` branch is intentionally behind that work. Its job is stability, not pretending to be current development.

## Roadmap

[`docs/ROADMAP.md`](docs/ROADMAP.md) records Phases 1–5 and the next bench milestones: board startup, CHC-104B-M2 characterization, HMI truth tables, RS-485 physical-layer resolution, motor-controller integration, flash validation, and controlled motion testing.

## Engineering standard

Project-owned code follows [`docs/CODING_STANDARD.md`](docs/CODING_STANDARD.md), informed by MISRA C:2023, CERT C, and JPL/NASA Power-of-Ten principles without claiming formal MISRA certification.

## License

Project-owned source is covered by **Mozilla Public License 2.0 (MPL-2.0)**. See [`LICENSE`](LICENSE) and [`docs/LICENSING.md`](docs/LICENSING.md). Imported STM32/CMSIS/FreeRTOS code keeps its upstream terms.

## Read these first

- [`docs/BRANCH_MODEL.md`](docs/BRANCH_MODEL.md)
- [`docs/ROADMAP.md`](docs/ROADMAP.md)
- [`docs/VALIDATION_LEVELS.md`](docs/VALIDATION_LEVELS.md)
- [`docs/CODING_STANDARD.md`](docs/CODING_STANDARD.md)
- [`docs/LICENSING.md`](docs/LICENSING.md)
- [`CONTRIBUTING.md`](CONTRIBUTING.md)

A build is evidence that software can become a binary. It is not evidence that the machine is safe to move a person.

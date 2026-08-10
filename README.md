# JoyStick Interface Firmware

Firmware for the Accessibilita joystick interface platform, built around the STM32F446VET6.

> **Branch: `edge` — hardware-facing integration baseline.** This branch intentionally holds the Phase-1 safety foundation while `experimental` carries newer software-only phases developed ahead of hardware availability.

**Raw joystick data is not a motor command.**

The Phase-1 tree established deterministic acquisition, diagnostics, explicit safety state, static RTOS ownership, watchdog supervision, and a hard physical-output lock before later software work was allowed to grow around it.

## Branch position

| Branch | Current role |
|---|---|
| `main` | stable / release-facing history |
| `edge` | Phase-1 safety foundation, ready for hardware bring-up |
| `experimental` | Phase 1–5 software model + full-system fault simulation |

See [`docs/BRANCH_MODEL.md`](docs/BRANCH_MODEL.md).

## What `edge` actually proves

This branch is a **target-built Phase-1 safety foundation**. It does not contain the later Phase-2 through Phase-5 runtime/protocol/configuration/system-simulation implementation.

The physical RS-485 path remains disabled. The known PE7/PE8 direction conflict and single-channel joystick diagnostic limitation remain hardware blockers to be measured, not software facts to be wished away.

## What exists ahead on `experimental`

`experimental` has software-validated:

- addressed motor protocol and link/session model
- joystick calibration/configuration/Q15 shaping
- runtime integration and HMI/operating-mode model
- deterministic full-system fault simulation
- logical drive authorization testing while physical output remains independently zero
- RX/WX ELF segment policy

That work is deliberately not presented here as hardware evidence.

## Roadmap and validation language

Read [`docs/ROADMAP.md`](docs/ROADMAP.md) and [`docs/VALIDATION_LEVELS.md`](docs/VALIDATION_LEVELS.md). The next major step for `edge` is hardware bring-up and measurement, not pretending the Phase-5 simulator replaced a bench.

## Engineering standard

Project-owned code follows [`docs/CODING_STANDARD.md`](docs/CODING_STANDARD.md).

## License

Project-owned source is covered by **Mozilla Public License 2.0 (MPL-2.0)**. See [`LICENSE`](LICENSE) and [`docs/LICENSING.md`](docs/LICENSING.md). Imported STM32/CMSIS/FreeRTOS source retains upstream terms.

## Firmware project

The STM32 project lives at:

```text
src/JoyStick_V2_FreeRTOS/
```

Read its README and `docs/` directory before hardware bring-up.

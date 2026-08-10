# JoyStick V2 Firmware — `edge`

This is the STM32F446 firmware project on the **hardware-facing `edge` branch**.

`edge` currently preserves the Phase-1 safety foundation. Newer software phases are on `experimental`; they have not been silently backported here and should not be inferred from repository-level roadmap text.

## What this branch contains

- deterministic dual-axis ADC/DMA acquisition
- input freshness and plausibility diagnostics
- explicit safety-state foundation
- static FreeRTOS application tasks/mailboxes
- task-health supervision
- Safety-Control-owned watchdog policy
- debugger snapshot
- forced-disabled physical RS-485 path
- Phase-1 invariant checks
- Debug and Release target builds

## Safety position

```text
physical motor transmission   absent / disabled
physical drive authorization  false
hardware qualification        not claimed
```

The MAX3535 / PE7/PE8 direction mismatch and the single-pot-per-axis joystick diagnostic limitation remain hardware facts to resolve on the bench.

## Why `experimental` is ahead

Phases 2–5 were developed as software models while the physical board was unavailable. They cover the motor protocol, calibration/configuration, runtime integration, HMI/operating modes, and whole-system fault simulation.

That software evidence is useful. It is not a substitute for bringing this `edge` baseline up on hardware and measuring the real system.

## Build baseline

```text
STM32CubeIDE 2.2.0
GNU Tools for STM32 14.3.rel1
arm-none-eabi-gcc 14.3.1
STM32CubeF4 V1.28.1 dependency baseline
```

Typical Phase-1 gates:

```bash
make phase1-check
make debug
make release
```

## Documentation

Repository-wide policy:

- `../../docs/BRANCH_MODEL.md`
- `../../docs/ROADMAP.md`
- `../../docs/VALIDATION_LEVELS.md`
- `../../docs/CODING_STANDARD.md`
- `../../docs/LICENSING.md`

Firmware-specific engineering records:

- `docs/PHASE1_ARCHITECTURE.md`
- `docs/IMPLEMENTATION_STATUS.md`
- `docs/HARDWARE_PINMAP.md`
- `docs/HARDWARE_BLOCKERS.md`
- `docs/DEPENDENCIES.md`
- `docs/STM32CUBEIDE_WORKFLOW.md`
- `docs/CUBEMX_REGENERATION.md`

The hardware has opinions. The bench gets the deciding vote.

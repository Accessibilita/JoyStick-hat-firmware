# Branch model

This repository uses three long-lived branches for three different kinds of truth.

The branch names are workflow labels. They are **not safety certifications**.

| Branch | Purpose | Promotion rule |
|---|---|---|
| `main` | stable / accepted history | receives milestones that have passed the acceptance level required for release |
| `edge` | serious integration and hardware-facing bring-up | receives coherent work that is ready for the next real hardware/integration gate |
| `experimental` | ahead-of-hardware engineering | carries the newest software architecture, models, simulations, and fault campaigns |

## `main`

`main` is deliberately conservative. It is the branch a new reader is most likely to land on, so its documentation must explain that newer work exists elsewhere.

A commit being on `main` does not mean the complete wheelchair system is hardware-qualified. Release notes must still state the actual validation level.

## `edge`

`edge` is the hardware-facing development baseline.

Today it contains the Phase-1 safety foundation. It is intentionally behind `experimental` because the later software phases were developed ahead of board availability.

When hardware work resumes, `edge` is where software and physical evidence start meeting each other.

## `experimental`

`experimental` means **ahead of hardware**, not sloppy.

The same coding standard applies. The difference is evidence: this branch is allowed to prove architecture, state machines, protocol behavior, configuration models, and fault handling in software before the physical board exists to validate them.

As of the Phase-5 milestone, `experimental` contains the complete Phase-1 through Phase-5 software model and deterministic full-system fault campaign while physical drive remains independently inhibited.

## Promotion flow

```text
experimental
    ↓ coherent + software gates pass
edge
    ↓ relevant bench / hardware gates pass
main
```

Promotion is evidence-driven. We do not merge a branch upward merely because enough time passed or because the code compiles.

## Validation labels

Documentation should use these labels consistently:

- **Implemented** — source exists.
- **Software-validated** — applicable invariants, host tests, sanitizers, analyzers, and simulation gates pass.
- **Target-built** — Debug/Release target images build with the controlled STM32 toolchain.
- **Hardware-validated** — the claimed behavior has been measured and accepted on real hardware.

If a claim does not say which level it reached, the claim is incomplete.

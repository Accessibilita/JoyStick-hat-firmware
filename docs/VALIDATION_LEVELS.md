# Validation levels and safety claims

This file exists because words like "done", "working", and "tested" are dangerously vague in embedded control software.

## Level 1 — Implemented

The source exists and is integrated into the intended architecture.

That is not proof of correctness.

## Level 2 — Software-validated

The applicable software gates pass. Depending on the phase, those gates include:

- source invariant checks
- strict warnings-as-errors builds for project-owned code
- deterministic host tests
- ASan + UBSan
- GCC `-fanalyzer`
- malformed-input campaigns
- full-system deterministic fault simulation

This proves software behavior only within the model and test assumptions.

## Level 3 — Target-built

The code builds into STM32 ELF/HEX/BIN images with the controlled toolchain.

The current controlled development baseline is STM32CubeIDE 2.2.0 with GNU Tools for STM32 14.3.rel1 / GCC 14.3.1 and the historical STM32CubeF4 V1.28.1 firmware dependency baseline.

A target build proves that the source is coherent enough to become a target binary. It does not prove that peripherals, timing, wiring, or electromechanical behavior match our assumptions.

## Level 4 — Hardware-validated

A claim reaches this level only when the relevant behavior has been measured and accepted on actual hardware using a documented test procedure.

Examples include oscilloscope captures, logic-analyzer traces, debugger observations, fault injection, bus measurements, and controlled bench operation.

## Level 5 — System-accepted

Reserved for a complete milestone whose required software and hardware evidence has been reviewed together.

The repository is **not** claiming this level for powered mobility operation today.

## Current project statement

The `experimental` branch is software-validated through Phase 5 and target-built. The physical drive path remains locked out, target configuration storage and final HMI drive-enable semantics remain unqualified, and complete hardware validation is not claimed.

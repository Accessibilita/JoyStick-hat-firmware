# Phase 3 Test Plan

Phase 3 is accepted as software only.

A green build does not validate the CHC-104B-M2, the ADC front end, STM32 flash, or the physical drive path.

## Required software gates

```text
Phase-1 invariants
Phase-2 invariants
Phase-3 invariants
Phase-1 host tests
Phase-2 host tests
Phase-3 host tests
ASan + UBSan
GCC -fanalyzer
STM32 Debug build
STM32 Release build
```

## Configuration-record tests

- CRC32 golden vector;
- encode/decode round trip;
- one-bit corruption rejected;
- reserved-field corruption rejected;
- semantic invalidity rejected;
- newest valid redundant slot selected;
- corrupted newer slot falls back to older valid slot;
- next-generation preparation increments exactly once;
- generation overflow rejected;
- random 64-byte records never bypass validation.

## Calibration tests

- bounded center capture;
- bounded sweep capture;
- center average generated only after target sample count;
- synthetic asymmetric CHC-104B-M2 sweep accepted;
- endpoint guard applied;
- center-noise-derived deadband never below policy floor;
- insufficient travel rejected;
- invalid ADC-domain samples rejected;
- invalid state transitions rejected.

## Processing tests

- center maps exactly to zero;
- counts inside deadband map exactly to zero;
- calibrated minimum maps to -32767;
- calibrated maximum maps to +32767;
- values beyond calibrated usable endpoints saturate rather than overflow;
- asymmetric sides scale independently;
- inversion reverses sign;
- linear response is identity;
- cubic response reduces mid-stick magnitude while retaining full scale;
- speed limiting is bounded;
- X maps to turn;
- Y maps to forward;
- invalid configuration produces an inhibited requested command;
- 20,000 deterministic random ADC pairs remain inside Q15 bounds.

## Hardware-deferred tests

These remain explicitly unproven:

- CHC-104B-M2 endpoint values;
- center noise and repeatability;
- real deadband requirement;
- ADC loading and sampling behavior;
- open-wiper behavior;
- flash erase/program/verification timing;
- power-loss behavior during actual flash write;
- real response-curve preference;
- real speed-limit behavior;
- any physical drive response.

# Phase 3 Implementation Status

## Implemented

- CHC-104B-M2 reference profile identifier;
- bounded center-capture calibration state;
- bounded sweep-capture calibration state;
- asymmetric per-axis calibration;
- guarded usable endpoints;
- center-noise-derived deadband with explicit software policy;
- signed Q15 normalization;
- axis inversion;
- linear-to-cubic integer response shaping;
- Q15 maximum-speed limiting;
- X-to-turn / Y-to-forward requested-command construction;
- fail-closed request construction;
- fixed 64-byte persistent configuration record;
- explicit little-endian serialization;
- CRC32/IEEE record protection;
- semantic record validation;
- redundant two-slot newest-valid selection;
- next-generation preparation;
- Phase-3 invariant checker;
- portable host tests and malformed-record campaign.

## Software validated by the package gate

The installation script requires all of these before it will commit:

- Phase-1 invariant checker;
- Phase-2 invariant checker;
- Phase-3 invariant checker;
- Phase-1 host tests;
- Phase-2 host tests;
- Phase-3 host tests;
- ASan + UBSan;
- GCC static analyzer;
- STM32 Debug target build;
- STM32 Release target build;
- staged `git diff --check`.

## Intentionally not integrated as runtime authority

Safety Control still publishes `configuration_valid = false`.

That is deliberate. We have defined and tested what a valid calibration/configuration record means, but the board has not yet loaded one from real hardware-backed storage and the CHC-104B-M2 has not been characterized.

The physical authorization boundary also remains zeroed and disabled.

## Hardware validation still required

- actual CHC-104B-M2 ADC center and endpoints;
- center-noise and return repeatability;
- deadband selection;
- direction/inversion confirmation;
- open-wiper/fault behavior;
- STM32 flash backend;
- flash brownout/torn-write behavior on the MCU;
- physical RS-485 link;
- motor-controller integration;
- any powered motion.

Phase 3 gives us a real configuration and command-shaping model. It does not turn missing measurements into facts.

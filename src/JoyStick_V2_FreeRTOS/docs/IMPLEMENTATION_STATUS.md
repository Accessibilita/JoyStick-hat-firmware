# Implementation status — `edge`

`edge` is the hardware-facing Phase-1 baseline.

It is deliberately less feature-rich than `experimental` and deliberately more conservative about what it claims.

## Implemented on this branch

- STM32F446VET6 startup/peripheral foundation
- 96 MHz HSE/PLL clock setup
- static Safety / RS-485 / HMI / Diagnostics tasks
- static queues/mailboxes
- TIM2-triggered ADC1 scan of PA0/PA1
- circular DMA with bounded sample batches
- acquisition sequence/freshness/race diagnostics
- joystick plausibility checks
- regulator power-good observation
- neutral qualification
- safety-state-machine foundation
- explicit authorized-command boundary
- task-health supervision
- Safety-Control-owned IWDG refresh
- debugger watchdog freeze for Debug
- forced-disabled RS-485 driver
- debugger snapshot
- Phase-1 host/invariant checks
- Debug/Release target build flow

## Not on this branch

The following later work exists on `experimental`, not `edge`:

- finalized software motor-protocol model
- motor-link session/sequence qualification
- calibration/configuration/Q15 shaping
- runtime configuration authority
- HMI operating-mode model
- full-system Phase-5 simulator/fault campaign
- Phase-5 linker RX/WX policy

## Hardware validation

Still pending:

- startup/reset on the actual board
- ADC voltage/count characterization
- TIM2/DMA timing on silicon
- CHC-104B-M2 center/noise/endpoints
- open/short fault behavior
- regulator PG timing
- watchdog reset behavior on target
- GPIO safe-state measurements
- physical RS-485 behavior
- motor-controller integration

## Known blockers

- PE7/PE8 physical RS-485/UART direction mismatch
- single-channel potentiometer sensing cannot distinguish every electrical failure from a valid endpoint command

## Next move for `edge`

Hardware bring-up.

The software model on `experimental` tells us what to measure. `edge` is where the measurements start becoming authoritative.

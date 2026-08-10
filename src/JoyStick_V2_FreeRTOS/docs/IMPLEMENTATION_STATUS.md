# Implementation status — `experimental`

`experimental` is currently at **Phase 5: full-system software fault simulation**.

The branch is allowed to get ahead of hardware, but it still follows the same coding and evidence rules as the hardware-facing branches.

## Implemented

### Phase 1 — safety foundation

- deterministic ADC/DMA joystick acquisition
- freshness/plausibility diagnostics
- explicit SafetyState
- static RTOS ownership
- task-health supervision
- Safety-Control-owned watchdog
- debugger snapshot
- physical RS-485 lockout

### Phase 2 — motor-link model

- fixed 32-byte command/status frames
- explicit source/destination addressing
- CRC16/CCITT-FALSE
- sessions and sequence acknowledgements
- MotorLinkState qualification/restart/timeout/fault handling
- fake motor controller
- logical authorization model with physical zero boundary

### Phase 3 — calibration/configuration/shaping

- CHC-104B-M2 reference profile
- asymmetric min/center/max calibration model
- guarded endpoints/deadband
- signed Q15 normalization
- integer response shaping
- speed limiting
- two-slot versioned CRC-protected configuration record model

### Phase 4 — runtime integration

- runtime configuration authority plumbing
- debounced HMI state model
- explicit operating modes
- real requested-command generation
- expanded debugger state
- target storage/HMI drive semantics intentionally fail-closed

### Phase 5 — system abuse

- deterministic system simulator using the real application modules
- logical drive authorization reachable in simulation
- physical command still independently zero/unauthorized
- controller restart and remote-fault requalification
- CRC/stale ACK/silence/timeout campaigns
- configuration/task/power/ADC fault injection
- calibration/service-mode zero ceilings
- reboot-with-displaced-stick cases
- 32-bit tick wrap test
- 20,000-step deterministic abuse campaign
- explicit ELF RX FLASH / RW RAM program-header policy

## Phase-5 software acceptance

```text
Phase 1–5 invariants                    PASS
Phase 1–5 host tests                    PASS
ASan + UBSan                            PASS
GCC -fanalyzer                          PASS
full-system fault campaign              PASS
STM32 Debug build                       PASS
STM32 Release build                     PASS
Debug/Release ELF no-RWX check          PASS
```

## Target footprint at Phase 5

```text
Debug   FLASH 21,300 B   RAM 9,856 B
Release FLASH 20,324 B   RAM 9,856 B
```

## Still not hardware-validated

- complete board startup/safe-state behavior
- CHC-104B-M2 real electrical/mechanical characteristics
- ADC/DMA timing on silicon
- HMI electrical mapping and final enable semantics
- STM32 flash transaction behavior under interruption/brownout
- physical RS-485 link
- motor-controller safe-state/timeout/brake behavior
- complete electromechanical system

## Current physical-output statement

```text
Physical drive:                 LOCKED OUT
Target configuration storage:   UNQUALIFIED / DISABLED
Target HMI drive-enable mapping: UNQUALIFIED / DISABLED
Hardware validation:            NOT CLAIMED
```

## Next meaningful milestone

Hardware validation.

The architecture has reached the point where more software can still be written, but several decisive answers belong to scopes, logic analyzers, debuggers, power interruption, the real CHC-104B-M2, the PCB, and the motor controller.

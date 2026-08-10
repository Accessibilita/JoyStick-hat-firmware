# Firmware roadmap

The project is being developed from the safety boundary outward.

A phase number describes engineering scope. It does not erase hardware-validation debt.

## Completed software phases

| Phase | Scope | Current evidence |
|---|---|---|
| 1 | safety foundation, ADC/DMA acquisition, diagnostics, RTOS ownership, watchdog, physical lockout | implemented + target-built; hardware validation pending |
| 2 | addressed motor protocol, CRC/session/sequence model, motor-link state, logical authorization model | software-validated on `experimental`; hardware transport pending |
| 3 | joystick calibration/configuration model, Q15 normalization, shaping, redundant record model | software-validated on `experimental`; real joystick/flash characterization pending |
| 4 | runtime integration, HMI model, operating modes, configuration authority plumbing | software-validated on `experimental`; target HMI semantics/storage backend intentionally unqualified |
| 5 | whole-system deterministic simulator, cross-module fault campaign, logical-vs-physical authorization split, ELF RX/RW policy | software-validated + target-built on `experimental`; hardware validation still pending |

## Current branch position

```text
main          stable historical / release-facing baseline
edge          Phase-1 hardware-facing safety foundation
experimental  Phase-1 through Phase-5 software architecture + fault simulation
```

See `BRANCH_MODEL.md` before assuming the newest phase belongs on every branch.

## Next milestone: hardware evidence

The next major milestone is not "Phase 6 = more features."

We now have enough software architecture that several unanswered questions belong to the bench:

1. **Board startup and safe-state measurements**
   - reset behavior
   - GPIO startup levels
   - watchdog behavior
   - regulator power-good timing

2. **CHC-104B-M2 characterization**
   - center counts and noise
   - usable endpoint counts
   - mechanical repeatability
   - deadband requirements
   - wiring direction / inversion
   - open/short fault behavior

3. **ADC/DMA timing on silicon**
   - trigger rate
   - batch timing
   - ISR/task latency
   - freshness limits

4. **HMI electrical truth**
   - button polarity and debounce behavior
   - rotary selector truth tables
   - LED-driver behavior
   - final mapping from user controls to mode/enable requests

5. **RS-485 physical-layer resolution**
   - resolve PE7/PE8 direction mismatch
   - confirm MAX3535 topology
   - confirm termination and daisy-chain behavior
   - prove safe DE/RE startup states

6. **Motor-controller bench integration**
   - addressing
   - session/restart handling
   - timeout behavior
   - brake/ready semantics
   - corrupted/stale frame handling

7. **Configuration storage on STM32 flash**
   - erase/program timing
   - readback
   - interrupted-write behavior
   - brownout behavior
   - endurance strategy

8. **Controlled motion validation**
   - only after the preceding gates pass
   - begin unloaded / restrained
   - verify command sign, magnitude, timeout, braking, and fault-stop behavior before human use is even discussed

## Hardware-validation debt that software cannot pay

- single-pot joystick fault ambiguity
- actual joystick noise/endpoints
- board-level RS-485 pin conflict
- physical bus topology/termination
- motor-controller safe-state behavior
- flash behavior under power loss
- real watchdog/reset timing
- HMI electrical mapping

Firmware does not get to vote inconvenient hardware facts out of existence.

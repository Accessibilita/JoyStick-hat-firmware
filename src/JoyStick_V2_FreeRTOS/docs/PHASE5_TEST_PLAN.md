# Phase 5 test plan

Phase 5 is an integration and fault-injection gate. Passing a compiler is necessary, but it is not the interesting part anymore.

## Mandatory full-system scenarios

The host simulator must prove all of the following with the real production modules linked into the test executable:

1. Cold boot qualifies motor link and neutral state before READY.
2. Logical `DRIVE_AUTHORIZED` is reachable only after a qualified neutral enable sequence.
3. Non-zero requested motion can exist while the physical command remains exactly zero and unauthorized.
4. Controller-session restart drops authorization and requires safety requalification.
5. A remote fault clears previous link qualification; one good frame is insufficient to recover READY.
6. CRC corruption invalidates the link and blocks authorization.
7. A stale acknowledgment invalidates the link and blocks authorization.
8. Controller silence exceeds the link timeout and transitions the link offline.
9. Invalid configuration suppresses requested motion.
10. Mandatory-task health failure blocks authorization.
11. Power-good loss is visible through input diagnostics and blocks authorization.
12. DMA overrun is visible through input diagnostics and blocks authorization.
13. Input timestamp staleness is detected with unsigned wrap-safe age math.
14. ADC rail/out-of-engineering-range input is rejected.
15. Calibration and service operating modes have a zero motion ceiling.
16. Firmware reboot with a displaced joystick and asserted enable cannot resume drive authorization.
17. Neutral and link timing remain correct across `uint32_t` millisecond wraparound.
18. A deterministic 20,000-step mixed-fault campaign never violates the physical-output invariant.

## Long campaign

The campaign varies:

- joystick X/Y samples, including occasional invalid rails;
- enable request;
- operating-mode requests and speed ceilings;
- configuration validity;
- mandatory-task health;
- power-good state;
- DMA overrun state;
- dropped motor responses;
- corrupt CRC;
- wrong session echo;
- stale ACK;
- controller restart;
- remote fault;
- remote-not-ready response.

A fixed xorshift32 seed makes failures reproducible. Random-looking input without reproducibility is just a hard-to-debug anecdote.

Every iteration checks the non-negotiable invariant:

```text
physical.drive_authorized == false
physical.forward_q15       == 0
physical.turn_q15          == 0
physical.maximum_speed_q15 == 0
```

It also verifies that logical authorization cannot coexist with a failed prerequisite.

## Static and build gates

Phase 5 retains every earlier gate and adds its own:

```text
make phase5-check
make host-test
make host-sanitize
make host-analyze
make BUILD=Debug target
make BUILD=Debug elf-check
make BUILD=Release target
make BUILD=Release elf-check
```

Owned code remains warnings-as-errors. ASan/UBSan exercise all host suites. GCC `-fanalyzer` includes the Phase-5 system harness.

## Hardware-validation boundary

A successful Phase-5 run means:

```text
IMPLEMENTED
SOFTWARE-VALIDATED
```

It does not mean:

```text
HARDWARE-VALIDATED
```

That label remains unavailable until the actual controller, joystick, PCB, bus, power system, and fault behavior are on the bench.

# Phase 4 Test Plan

Phase 4 is software validation. Hardware validation is not claimed.

## Host gates

The Phase-4 host suite must prove:

- a valid Phase-3 configuration record can become active runtime configuration;
- a missing or invalid storage slot cannot become valid by accident;
- short button/selector glitches do not become stable HMI state;
- target HMI semantics remain fail-closed;
- calibrated ADC input reaches the Phase-3 Q15 processing path;
- operating-mode/profile transitions require the reviewed prerequisites;
- fail-closed SafetyState startup faults cannot poison a valid first-cycle operating-mode decision;
- calibration/service/fault modes have a zero speed ceiling;
- HMI/mode speed policy can reduce but never increase the stored speed ceiling;
- a non-zero requested command can be produced in the software model;
- the final physical command remains zero and unauthorized;
- earlier Phase-1, Phase-2, and Phase-3 tests continue to pass.

## Tool gates

```text
phase1-check
phase2-check
phase3-check
phase4-check
host-test
host-sanitize
host-analyze
Debug target build
Release target build
```

Owned code remains warnings-as-errors.

## Hardware debt

Still not validated:

- CHC-104B-M2 measured center/noise/endpoints;
- final button semantics;
- final rotary semantics;
- LED driver behavior;
- STM32 flash slot layout and erase/program behavior;
- physical RS-485;
- motor-controller interoperability;
- any powered traction behavior.

# Phase 5 architecture — full-system simulation and fault campaign

Phase 5 is where the pieces stop getting graded one at a time.

Phases 1 through 4 proved the safety state machine, motor-link protocol, joystick calibration/shaping, runtime integration, and HMI/configuration boundaries separately. Phase 5 builds a deterministic software wheelchair around those pieces and starts abusing the complete path.

The physical output remains locked out.

That is not a contradiction. It is the point.

## System under test

The host simulator executes the real project-owned logic:

```text
synthetic joystick / power / acquisition state
                  ↓
         InputDiagnostics
                  ↓
       RuntimeControl + modes
                  ↓
          SafetyState
                  ↓
      CommandAuthorization
                  ↓
       logical authorization
                  ↓
     Motor Protocol V1 frames
                  ↓
     Fake Motor Controller
                  ↓
        MotorLinkState
                  └─────────────── feedback to next control iteration

physical command returned by CommandAuthorization = always zero / unauthorized
```

The simulator does not replace these modules with look-alikes. It links the production `App/Src` implementations into the host test binary.

## Logical authorization is allowed to exist

Earlier phases deliberately tied `APP_SAFETY_DRIVE_AUTHORIZED` to the physical-link compile switch. That was useful while the architecture was still being poured.

It is too restrictive for system simulation.

Phase 5 separates the two concepts:

```text
SafetyState:             may prove logical DRIVE_AUTHORIZED
CommandAuthorization:    may prove logical_authorized = true
Physical command:        still hard-zeroed and unauthorized
RS-485 target transport: still compile-time blocked
UART target driver:      still absent
```

Raw joystick data is not a motor command. A logical authorization decision is not a GPIO pin either.

This lets us exercise the complete decision path without pretending the current PCB can drive the motor-controller link.

## Deterministic scheduler model

The host simulator advances in fixed 5 ms control steps and models the link service at 10 ms.

Periodic scheduling uses elapsed-time accumulation rather than `tick % period`. That detail matters because a 32-bit millisecond clock wraps and `2^32` is not an integer multiple of 10 ms. The simulator has to respect the same wrap-safe unsigned-time assumptions as the firmware or it becomes the bug.

No simulator loop is unbounded. `Phase5SystemSimulator_RunForMs()` rejects durations beyond its fixed test ceiling.

## Motor-link requalification

A remote-controller fault now clears the valid-frame qualification count.

Before this change, a remote fault could leave the old consecutive-valid count intact. One subsequent good status packet could therefore inherit pre-fault confidence and immediately make the link READY again.

Phase 5 changes that rule:

```text
qualified link
    ↓
remote fault
    ↓
FAULT + valid-frame count = 0
    ↓
first clean status
    ↓
SYNCHRONIZING
    ↓
second clean status
    ↓
READY
```

Old confidence does not survive a fault boundary.

## Reboot behavior

A simulated firmware reboot creates a new command session while leaving external reality alone:

- joystick position does not magically return to center;
- HMI enable state does not magically release;
- the remote controller remains the same physical controller;
- the stored configuration remains what it was.

The system must rebuild link qualification and neutral qualification from that state. A displaced joystick at reboot cannot resume logical drive authorization.

## Linker hygiene

Phase 5 also closes the remaining RWX ELF warning.

The linker script now declares explicit program headers:

```text
FLASH LOAD: read + execute
RAM LOAD:   read + write
```

The acceptance gate runs `readelf -lW` on both Debug and Release ELF files and rejects any writable+executable `LOAD` segment.

We do not silence the warning. We make the ELF layout say what the hardware architecture already means.

## Still intentionally blocked

Phase 5 does **not** claim:

- valid physical RS-485 data wiring on the current PCB;
- validated motor-controller bus topology;
- validated HMI button/rotary semantics;
- validated STM32 flash persistence behavior;
- measured CHC-104B-M2 calibration values;
- hardware watchdog/fault timing;
- hardware-safe motion.

The target `ConfigurationStorage` backend remains unavailable. The target HMI mapping remains invalid. The target link task still forces the transceiver safe-disabled.

The software model is getting much more complete. The hardware still gets the final vote.

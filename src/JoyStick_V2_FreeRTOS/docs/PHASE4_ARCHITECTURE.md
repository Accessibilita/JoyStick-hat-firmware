# Phase 4 — Runtime Integration and HMI

Phase 1 built the safety foundation. Phase 2 built the motor-link contract. Phase 3 built calibration, configuration records, and joystick shaping.

Phase 4 connects those pieces into the actual application path.

Raw joystick data is still not a motor command.

```text
ADC / DMA
    ↓
input diagnostics
    ↓
validated configuration
    ↓
calibration + Q15 shaping
    ↓
RequestedDriveCommand
    ↓
safety state
    ↓
command authorization
    ↓
physical command boundary
    ↓
ZERO / UNAUTHORIZED
```

The important change is that `SafetyControlTask_Run()` no longer manufactures a placeholder zero command before the useful software can run. It calls `RuntimeControl_Step()`, which builds the requested command and runs the real safety/authorization chain.

The important non-change is that `CommandAuthorization_Evaluate()` still owns the physical boundary. Phase 4 can prove a non-zero requested command in host tests. It cannot turn that into physical motion.

## Configuration authority

`ConfigurationRuntime_Load()` reads both Phase-3 record slots through a storage callback and delegates record validity to the existing decoder. CRC, version, generation, reserved fields, and calibration semantics remain one contract.

The STM32 storage backend deliberately returns unavailable in Phase 4. We have not validated the flash allocation, erase granularity, power-loss behavior, or read-back procedure on hardware. Firmware does not get to vote those facts out of existence.

## HMI authority

`HmiModel_Step()` debounces the four-button / rotary raw state and publishes current HMI state through a one-element overwrite mailbox.

Final button and selector semantics are not guessed. On the target build:

```text
enable_request        = false
control_mapping_valid = false
```

Host tests may inject a validated HMI state to prove downstream behavior. The target may not claim that mapping until the physical controls are characterized.

## Ownership

- HMI task owns raw HMI debounce and publication.
- Safety Control owns the runtime safety decision.
- configuration decoder owns configuration validity.
- command authorization owns the physical drive boundary.
- RS-485 task remains the eventual sole transport owner.

No task gets to manufacture drive authority because it happens to have useful information.

## Debug snapshot

The debugger mirror now exposes raw input, diagnostics, HMI state, active configuration, processed joystick axes, requested command, safety state, authorization blocks, logical authorization, and the final physical command.

This is observation only. Runtime decisions never read the debugger mirror.

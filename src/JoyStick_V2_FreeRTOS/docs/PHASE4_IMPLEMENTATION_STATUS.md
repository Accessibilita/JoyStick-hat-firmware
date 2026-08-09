# Phase 4 Implementation Status

## Implemented

- runtime configuration loader abstraction;
- two-slot configuration selection wired into runtime authority;
- debounced HMI raw-state model;
- explicit BOOT/NORMAL/REDUCED/PRECISION/CALIBRATION/SERVICE/FAULT operating-mode model;
- neutral-qualified drive-profile transitions and zero-ceiling service/fault modes;
- static RTOS HMI latest-state mailbox;
- integrated raw-input → calibration/shaping → requested-command path;
- HMI maximum-speed ceiling applied only as a reduction;
- safety observation generated from the integrated runtime path;
- Phase-2 command authorization called from runtime integration;
- expanded debugger snapshot;
- explicit newlib ENOSYS stubs for unsupported file operations;
- Phase-4 invariant checker;
- host tests, sanitizer path, and analyzer path;
- mandatory licensing/coding-standard header rule for Phase-4 owned source.

## Intentionally inhibited

- target HMI drive-enable mapping;
- target configuration flash reads/writes;
- physical RS-485;
- physical drive authorization.

## Software validated

The installer performs the real acceptance run on the developer workstation before commit/push. Until that run succeeds, the package itself is not evidence that the STM32 target build passes.

## Hardware validated

Nothing new in Phase 4.

The branch remains ahead of the hardware. That is the point of `experimental`, but the distinction stays explicit.

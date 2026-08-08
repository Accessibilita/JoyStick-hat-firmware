# Phase 1 architecture

## Safety principle

A requested command is not an authorized command. Only the Safety Control Task may publish an
`AppAuthorizedDriveCommand`, and Phase 1 intentionally prevents authorization.

## Tasks

| Task | Period/event | Priority | Phase 1 responsibility |
|---|---:|---:|---|
| Safety Control | ADC DMA notification / 5 ms timeout | 5 | Input diagnosis, state machine, neutral command publication, watchdog decision |
| RS-485 Link | 10 ms | 4 | Publish invalid link status; keep physical driver disabled |
| HMI | 20 ms | 2 | Maintain LED outputs in safe blank state |
| Diagnostics | 100 ms | 1 | Health progress and future deferred diagnostics |

All tasks and queues are statically allocated. No application heap allocation is permitted.

## Safety-control execution order

1. Wait for a completed ADC DMA half-buffer, with a 5 ms maximum wait.
2. Copy the completed sample block while DMA writes the opposite half.
3. Reduce the block to one bounded raw input snapshot.
4. Run electrical and freshness diagnostics.
5. Read the latest motor-link status snapshot.
6. execute the safety-state transition.
7. Publish a neutral, non-authorized command.
8. verify mandatory task-health progress.
9. refresh the independent watchdog only when RTOS progress is credible.

## Concurrency rules

- Interrupt routines only record completion and notify a task.
- The ADC DMA buffer uses completed-half ownership; the task never reads the half DMA is writing.
- Inter-task data uses length-one overwrite queues. Stale commands cannot accumulate.
- No mutex is taken by the Safety Control Task.
- No logging, string formatting, flash access, or peripheral wait occurs in the Safety Control Task.

## JTAG behavior

Debug builds call `__HAL_DBGMCU_FREEZE_IWDG()` before the watchdog starts. This permits source
stepping while preserving the release-build watchdog architecture. The release build does not
freeze the watchdog.

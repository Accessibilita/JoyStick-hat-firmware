# Phase 1 Architecture

Phase 1 is built around one rule:

**Requested motion is not authorized motion.**

That sounds obvious until firmware starts passing raw control values through three layers of callbacks and eventually nobody can point at the place where the machine actually decided it was safe to move.

We are not doing that.

There is one explicit authorization boundary and Safety Control owns it.

## Control path

```text
Raw inputs
    ↓
Input diagnostics
    ↓
Safety observation
    ↓
Safety state machine
    ↓
Requested command
    ↓
Authorization
    ↓
AuthorizedDriveCommand
```

Phase 1 deliberately stops before motion authorization.

The authorized command remains neutral and `drive_authorized` remains false.

## Tasks

| Task | Trigger / period | Priority | Job |
|---|---:|---:|---|
| Safety Control | DMA notification / 5 ms timeout | 5 | Inputs, diagnostics, safety state, authorization, task-health decision, watchdog |
| RS-485 Link | 10 ms | 4 | Own the future motor link; Phase 1 reports invalid link and keeps TX disabled |
| HMI | 20 ms | 2 | Maintain safe UI state and eventually process user requests |
| Diagnostics | 100 ms | 1 | Deferred diagnostics and low-priority observability |

The names matter less than the ownership.

Safety Control is the only application task allowed to decide that motion can be authorized.

## Safety-loop execution

The normal safety pass is:

1. Wait for a completed ADC DMA batch, with a bounded timeout.
2. Copy the completed samples while DMA is working somewhere else.
3. Verify that the source did not change underneath the copy.
4. Reduce the batch to the raw input snapshot used by the safety loop.
5. Check range, freshness, acquisition health, and 3.3 V power-good.
6. Read the most recent link-state snapshot.
7. Advance the safety state machine.
8. Construct the authorized command.
9. Publish the newest command through a one-element overwrite mailbox.
10. Check mandatory task progress.
11. Refresh IWDG only if the application still looks alive enough to deserve it.

There is intentionally no logging, flash write, dynamic allocation, protocol parsing, or random peripheral wait in the middle of that path.

## ADC and DMA ownership

TIM2 provides the acquisition cadence.

ADC1 scans PA0 and PA1.

DMA runs circularly.

The ISR does the minimum useful work: record completion state and wake Safety Control.

The safety task does not sit inside an interrupt trying to filter joystick samples or make state-machine decisions.

Sequence tracking exists because a DMA race that happens once every ten thousand cycles is still a race.

## Inter-task data

Control-state messages are snapshots, not historical events.

For that reason the firmware uses single-element overwrite mailboxes for the latest command and link state.

If five steering commands are published before the consumer wakes up, the consumer needs number five. It does not need to dutifully replay four stale commands first.

## Watchdog ownership

The independent watchdog is not a distributed participation trophy.

Only Safety Control refreshes it.

Mandatory tasks report progress through the health monitor. Safety Control looks at the system as a whole and decides whether the watchdog should be refreshed.

If the firmware stops making credible progress, the correct behavior is to stop proving to the watchdog that everything is fine.

## Debug behavior

Debug builds freeze IWDG while the core is halted under the debugger. That makes source stepping possible without deleting the watchdog architecture from the firmware.

Release builds do not freeze it.

The debugger snapshot:

```text
g_app_phase1_debug_snapshot
```

exists so the important state can be inspected without turning the safety loop into a printf demo.

## Phase-1 boundary

This architecture is intentionally useful before it is useful for motion.

That is the point.

We can verify acquisition, timing, state transitions, task health, watchdog behavior, GPIO safe states, and debugger visibility while the physical drive path is still locked out.

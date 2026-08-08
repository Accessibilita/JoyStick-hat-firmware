# Project coding standard

The application code follows a conservative embedded-C subset informed by MISRA C, CERT C, and
safe embedded-system practice.

Mandatory rules for hand-owned `App/` and `Platform/` code:

- ISO C11 syntax, fixed-width integer types, explicit units in names.
- No recursion, variable-length arrays, `goto`, `setjmp`, or `longjmp`.
- No dynamic allocation after reset; Phase 1 uses no application dynamic allocation at all.
- Every task and queue is statically allocated.
- Every non-void return value is checked or explicitly documented as intentionally ignored.
- Every loop is bounded except top-level RTOS task loops, which block or delay every iteration.
- Interrupt handlers do not parse protocols, filter signals, format text, or block.
- `volatile` is not used as a substitute for synchronization.
- One writer owns each published state object.
- Motion data uses overwrite semantics, never an accumulating FIFO.
- Safety-critical functions document preconditions, failure behavior, units, and timing intent.
- Production failures move outputs toward an inhibited state and then permit watchdog reset.
- Compiler warnings are treated as errors for hand-owned code.

Generated ST and third-party code is kept in `Core/`, `Drivers/`, and `Middlewares/`. Deviations
in vendor code do not relax the standard for `App/` or `Platform/`.

# Firmware Coding Rules

The point of the coding standard is not to make the source look ceremonial.

It is to make safety-relevant embedded code boring to misunderstand.

Application-owned code follows a conservative C subset informed by MISRA C:2023, CERT C, and the general "keep the state space small" philosophy behind the JPL/NASA Power of Ten rules.

## Code we own

The strict rules apply primarily to:

```text
App/
Platform/
Core/
```

where the file is application-owned rather than imported vendor code.

## Basic rules

- C11
- fixed-width integer types where width matters
- units in variable names when ambiguity would be dangerous
- no recursion
- no variable-length arrays
- no `goto`
- no `setjmp` / `longjmp`
- no application dynamic allocation after initialization
- Phase 1 uses static application allocation throughout
- bounded loops except the outer RTOS task loops
- explicit handling of meaningful return values
- minimal interrupt handlers
- no `volatile` pretending to be a synchronization primitive
- one clear writer for published state
- safety behavior documented where the reason is not obvious

## Safety-path rules

Safety Control does not:

- take a mutex
- format strings
- write nonvolatile storage
- wait on a peripheral
- parse a communications protocol
- perform unbounded work
- replay stale motion commands from a FIFO

The safety path needs predictable work and predictable ownership.

## Commands are state, not history

Motion data uses overwrite semantics.

If the producer publishes:

```text
10%
20%
30%
40%
```

before the consumer runs again, the consumer generally needs the current 40% state.

It does not need to execute 10, then 20, then 30 because a queue happened to save them.

## Interrupts

ISRs acknowledge hardware, capture the minimum necessary state, and notify the task that owns the real work.

Signal processing, protocol parsing, diagnostics, and safety-state transitions belong outside the interrupt context.

## Failure behavior

Production failures should move the system toward an inhibited output state.

Where recovery cannot be proven, the watchdog exists to reset the system rather than letting half-alive firmware keep control indefinitely.

## Compiler policy

Warnings are errors for application-owned code.

Vendor code gets a compatibility warning policy because importing somebody else's historical HAL is not a reason to weaken our rules, and fixing thousands of upstream formatting/style issues is not a useful firmware feature either.

Keep those two worlds separate.

## Owned-file header policy

New or materially rewritten project-owned C/H files state two things in the file header:

- the repository's actual licensing status; and
- that the file follows the GhostPCB firmware coding rules informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.

Do not invent an SPDX identifier. At Phase 4 this repository has no project-level `LICENSE`, so new owned files say that explicitly. Vendor sources keep their upstream notices and are not rewritten to look project-owned.

Comments are part of the engineering record. Safety-relevant code should explain ownership, failure behavior, units, state transitions, and why a guard exists when the reason is not obvious. Comments should explain the system rather than narrate C syntax.

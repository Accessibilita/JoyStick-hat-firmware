# Firmware coding standard

The point of this standard is not to make the source look ceremonial.

It is to make safety-relevant embedded code boring to misunderstand.

Project-owned firmware uses a conservative C11 subset informed by **MISRA C:2023**, **CERT C**, and the bounded-state / simple-control-flow philosophy behind the **JPL/NASA Power of Ten** rules. These references inform the GhostPCB rules; this project does not claim formal MISRA compliance or certification merely because the rules are named here.

## Code we own

The strict rules apply to project-owned code in:

```text
App/
Platform/
Core/
tests/host/
tools/
```

`Core/` includes board/project integration code generated or adopted into this project. If a future regeneration restores upstream notices or generated sections, those notices are preserved and the owned-code boundary is reviewed rather than blindly overwritten.

Imported dependencies remain separate:

```text
Drivers/
Middlewares/
```

Vendor code keeps its upstream license and compatibility warning policy. Somebody else's historical HAL is not a reason to weaken the rules for code we own.

## Mandatory embedded-C rules

- ISO C11.
- Fixed-width integer types where width matters.
- Units in identifiers where ambiguity could affect behavior.
- No recursion.
- No variable-length arrays.
- No `goto`.
- No `setjmp` / `longjmp`.
- No application dynamic allocation after initialization; the current application uses static RTOS allocation throughout.
- Bounded loops except outer RTOS task loops, which must block or delay every iteration.
- Explicit handling of meaningful return values.
- Minimal interrupt handlers.
- No `volatile` pretending to be synchronization.
- One clear writer for each published state object.
- Motion state uses overwrite/current-state semantics rather than a backlog of stale commands.
- Safety behavior is commented where the reason is not obvious.
- Production failures move the system toward inhibition; unrecoverable failures may rely on watchdog reset only after the output path is safe.

## Safety-path rules

Safety Control does not:

- take a mutex;
- format strings;
- write nonvolatile storage;
- wait on a peripheral;
- parse protocol frames;
- perform unbounded work;
- replay stale motion commands from a FIFO.

The control path has to stay predictable enough that timing and ownership can actually be reasoned about.

## Requested motion is not authorized motion

The architecture keeps these concepts separate:

```text
raw joystick state
      ↓
requested command
      ↓
logical safety authorization
      ↓
physical authorization boundary
```

Phase 5 deliberately allows the software model to reach logical authorization so the complete decision path can be tested. The physical command remains independently hard-inhibited while the present hardware blockers exist.

Firmware does not get to vote inconvenient hardware facts out of existence.

## Interrupts

ISRs acknowledge hardware, capture the minimum necessary state, and notify the task that owns the real work.

Signal processing, protocol parsing, diagnostics, HMI policy, and safety-state transitions stay outside interrupt context.

## Compiler and analysis policy

Project-owned target code builds with strict warnings treated as errors, including conversion, shadowing, format, and prototype diagnostics.

Host acceptance additionally runs:

- the complete invariant chain;
- deterministic host tests;
- ASan + UBSan;
- GCC `-fanalyzer`;
- the Phase-5 full-system fault campaign.

Vendor code gets a compatibility warning policy instead of project `-Werror`. These are two different worlds and the Makefile keeps them separate.

## License/header policy

Project-owned source is licensed under **Mozilla Public License 2.0** and carries:

```text
SPDX-License-Identifier: MPL-2.0
```

Owned C/H source headers also identify this coding standard. Python and shell support code carries equivalent comment-form provenance after the shebang when one exists.

Third-party source is not rewritten to look project-owned. Existing upstream copyright and license notices remain intact.

## Comments

Comments are part of the engineering record.

Good comments explain:

- why a guard exists;
- who owns a state transition;
- what failure does;
- what units and timing assumptions matter;
- why an apparently simpler implementation was rejected;
- what remains software-only or hardware-unvalidated.

Bad comments narrate syntax the compiler already makes obvious.

A build is evidence that the software is internally coherent enough to become a binary. It is not evidence that the complete electromechanical system is safe.

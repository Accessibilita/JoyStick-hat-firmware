# Firmware coding standard

The point of this standard is not to make the source look ceremonial.

It is to make safety-relevant embedded code boring to misunderstand.

Project-owned firmware uses a conservative C11 subset informed by **MISRA C:2023**, **CERT C**, and the bounded-state / simple-control-flow philosophy behind the **JPL/NASA Power of Ten** rules. These references inform the GhostPCB rules; this repository does not claim formal MISRA compliance or certification merely because the standards are named here.

## Scope

The strict rules apply to project-owned code and build logic, primarily:

```text
src/JoyStick_V2_FreeRTOS/App/
src/JoyStick_V2_FreeRTOS/Platform/
src/JoyStick_V2_FreeRTOS/Core/
src/JoyStick_V2_FreeRTOS/tests/host/
src/JoyStick_V2_FreeRTOS/tools/
project-owned Makefiles / linker scripts
```

Imported dependencies are a separate world:

```text
Drivers/
Middlewares/
```

Vendor code keeps its upstream license and compatibility warning policy. Somebody else's historical HAL is not a reason to weaken the rules for code we own, and it is not useful to rewrite upstream source merely to make it look locally styled.

## Mandatory embedded-C rules

- ISO C11.
- Fixed-width integer types where width matters.
- Units in identifiers where ambiguity could affect behavior.
- No recursion.
- No variable-length arrays.
- No `goto`.
- No `setjmp` / `longjmp`.
- No application dynamic allocation after initialization; current application architecture uses static RTOS allocation.
- Bounded loops except outer RTOS task loops, which must block or delay every iteration.
- Explicit handling of meaningful return values.
- Minimal interrupt handlers.
- No `volatile` pretending to be synchronization.
- One clear writer for each published state object.
- Current motion/state uses overwrite semantics rather than a FIFO backlog of stale commands.
- Safety behavior is commented where the reason is not obvious.
- Production failures move the system toward inhibition.
- Unrecoverable failures may rely on watchdog reset only after the output path is safe.

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

## Requested motion is not physical authority

The architecture keeps these concepts separate:

```text
raw joystick state
      ↓
requested command
      ↓
logical safety authorization
      ↓
physical authorization boundary
      ↓
physical transport
```

Host simulation is allowed to reach logical authorization. Hardware output remains independently gated until the physical system earns it.

## Interrupts

ISRs acknowledge hardware, capture the minimum necessary state, and notify the task that owns the real work.

Signal processing, protocol parsing, diagnostics, HMI policy, and safety-state transitions stay outside interrupt context.

## Compiler and analysis policy

Project-owned target code builds with strict diagnostics treated as errors, including conversion, shadowing, format, and prototype checks.

The advanced acceptance chain on `experimental` includes:

- complete phase invariant chain
- deterministic host tests
- ASan + UBSan
- GCC `-fanalyzer`
- whole-system deterministic fault simulation
- Debug and Release STM32 target builds
- explicit ELF segment-permission checks

Older branches may not contain every later-phase test tool. Their documentation must say which gates actually exist there instead of pretending history has been backported.

## Source comments

Comments are part of the engineering record.

Good comments explain:

- why a guard exists;
- who owns a state transition;
- what failure does;
- what units and timing assumptions matter;
- why an apparently simpler implementation was rejected;
- what remains software-only or hardware-unvalidated.

Bad comments narrate syntax the compiler already makes obvious.

## License/header policy

Project-owned source is covered by **Mozilla Public License 2.0** at repository level.

New and materially modified project-owned source should carry:

```text
SPDX-License-Identifier: MPL-2.0
```

and should identify this coding standard where the file format permits a useful header.

`experimental` received broad SPDX/header coverage during Phase 5. `main` and `edge` preserve their historical source snapshots; this documentation convergence intentionally does not rewrite hundreds of old source files merely to make the diff look uniform. The root `LICENSE` and licensing documentation define the project-owned license boundary on those branches.

Third-party source is never re-labeled as project-owned.

## The rule behind the rules

A build is evidence that the software is internally coherent enough to become a binary.

It is not evidence that the complete electromechanical system is safe.

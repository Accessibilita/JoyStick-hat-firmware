# Contributing

This is safety-relevant embedded control software. Contributions are welcome, but "it works on my machine" is not the acceptance standard.

## Pick the correct branch

- Start ahead-of-hardware architecture, simulations, and new software phases from `experimental`.
- Hardware bring-up and integration intended for the current physical baseline belongs on `edge` once the software is coherent enough for that gate.
- Do not target `main` directly for speculative work.

Read `docs/BRANCH_MODEL.md` first.

## Code we own

Project-owned code follows `docs/CODING_STANDARD.md`.

That includes strict warnings, bounded behavior, fixed-width types where widths matter, static RTOS allocation, minimal ISRs, explicit failure handling, and comments that explain safety intent rather than narrating syntax.

New/materially modified project-owned source should carry the MPL-2.0 SPDX identifier.

## Evidence belongs with the change

A useful contribution says what was actually proven:

```text
IMPLEMENTED
SOFTWARE-VALIDATED
TARGET-BUILT
HARDWARE-VALIDATED
```

Do not upgrade the label because a test sounds convincing.

Hardware claims should include a repeatable procedure and evidence such as scope captures, logic traces, measurements, or debugger observations.

## Generated/vendor code

Do not mass-format or relicense `Drivers/` or `Middlewares/` to make a patch look consistent. Keep the project-owned and imported worlds separate.

## Commit style

Keep commit subjects short and useful. Put the engineering story in the documentation and patch, not in a paragraph-long commit title.

# CubeMX regeneration policy

The original repository's `.ioc` conflicted with the uploaded KiCad netlist on PA4, PA6,
PC8/PC9, and PE7/PE8. For that reason, generated files cannot be treated as correct merely
because CubeMX produced them.

During Phase 1:

- `Core/`, `App/`, and `Platform/` in this package are the compile authority.
- The `.ioc` records the intended migration direction.
- Generate only into a disposable staging directory.
- Compare the generated pin map against `docs/HARDWARE_PINMAP.md`.
- Preserve the external Makefile and application directories.
- Never accept a regenerated UART5 configuration for the current PCB revision.
- Never accept dynamic FreeRTOS objects or the old nine placeholder tasks.

After the `.ioc` has been opened, validated, regenerated, built, and tested on the actual board,
it can be promoted to regeneration authority in a separately reviewed commit.

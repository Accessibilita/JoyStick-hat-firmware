# CubeMX Regeneration Policy

The `.ioc` is not currently allowed to overrule the board.

The historical project contained pin/peripheral assignments that conflict with the reviewed KiCad design, including PA4, PA6, PC8/PC9, and the PE7/PE8 motor link.

So "CubeMX generated it" is not sufficient evidence that it is correct.

## Current authority

During Phase 1:

```text
reviewed board/netlist
        ↓
documented hardware pin map
        ↓
accepted Core/App/Platform source
```

The `.ioc` records the intended configuration, but it has not yet earned the right to regenerate the accepted tree blindly.

## Regeneration workflow

If CubeMX regeneration is needed:

1. generate into a disposable directory
2. compare the resulting pin map to `HARDWARE_PINMAP.md`
3. review clocks and peripheral modes
4. review every changed source file
5. preserve the project-owned Makefile
6. preserve `App/` and `Platform/`
7. do not resurrect the old placeholder FreeRTOS task layout
8. do not enable the invalid UART5 PE7/PE8 motor-link configuration
9. build and test the merged result
10. only then move accepted changes into the real tree

## When can the `.ioc` become authority?

After it has been:

- reviewed against the actual PCB
- regenerated with the intended CubeMX version
- diffed against the accepted source
- built successfully
- run on the target board
- tested through the relevant hardware paths

At that point we can promote it in a deliberate commit.

Until then, regeneration is a tool.

It is not the source of truth.

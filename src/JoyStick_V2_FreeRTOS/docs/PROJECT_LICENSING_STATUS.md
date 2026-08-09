# Project Licensing Status

As of Phase 4, this repository does not contain a project-level `LICENSE` file and no project SPDX identifier was found in the owned firmware source.

That means Phase-4 owned C/H files state the situation directly instead of inventing a license:

```text
License: No project license is declared in this repository at Phase 4.
Do not assume permission to redistribute this project-owned file.
```

Third-party STM32, CMSIS, and FreeRTOS sources retain their own upstream licensing terms and are not rewritten to carry the project-owned header.

Before a public release intended for reuse or redistribution, choose the project license deliberately, add the repository license text, and update the owned-file header convention to the correct SPDX identifier.

This is a release/documentation blocker, not a runtime safety mechanism.

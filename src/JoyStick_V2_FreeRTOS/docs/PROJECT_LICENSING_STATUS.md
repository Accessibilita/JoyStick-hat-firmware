# Project licensing status

Phase 5 establishes **Mozilla Public License 2.0 (MPL-2.0)** as the license for project-owned JoyStick Interface firmware source.

The repository root carries the complete `LICENSE` text. Project-owned source files use:

```text
SPDX-License-Identifier: MPL-2.0
```

The point is to make the boundary obvious. Code we own is open under a real, standard license. Imported code does not get scrubbed and re-labeled just because it lives in the same firmware tree.

## Project-owned MPL-2.0 scope

The Phase-5 header audit covers project-owned code in:

```text
src/JoyStick_V2_FreeRTOS/App/
src/JoyStick_V2_FreeRTOS/Platform/
src/JoyStick_V2_FreeRTOS/Core/
src/JoyStick_V2_FreeRTOS/tests/host/
src/JoyStick_V2_FreeRTOS/tools/
```

These files also identify the GhostPCB firmware coding standard in their headers.

## Third-party code keeps its own license

The project does **not** replace upstream notices in imported dependencies. In particular:

```text
Drivers/
Middlewares/
```

contain ST, CMSIS, FreeRTOS, or other third-party material whose upstream terms remain authoritative for those files.

MPL-2.0 is file-level copyleft. That is useful here: modifications to MPL-covered project files stay under MPL when distributed, while those files can still be combined with separately licensed components in a larger work.

Licensing is now an explicit repository property instead of a TODO hidden in a source comment.

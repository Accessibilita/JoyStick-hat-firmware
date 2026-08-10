# Licensing

Project-owned JoyStick Interface firmware is released under the **Mozilla Public License 2.0 (MPL-2.0)**.

The complete license text is in the repository-root `LICENSE` file.

## Project-owned scope

The MPL-2.0 declaration applies to project-owned source, tests, build logic, and documentation unless a file states otherwise.

The advanced `experimental` branch uses `SPDX-License-Identifier: MPL-2.0` broadly in project-owned source. Historical source snapshots on `main` and `edge` may predate that header rollout; the absence of an SPDX line in an older project-owned file does not mean the repository-root license was replaced by some third-party license.

## Third-party code

Imported material keeps its upstream terms and notices.

In particular, do not treat these trees as if Accessibilita relicensed them:

```text
src/JoyStick_V2_FreeRTOS/Drivers/
src/JoyStick_V2_FreeRTOS/Middlewares/
```

Those directories contain STM32 HAL, CMSIS, FreeRTOS, and related upstream material. Their own copyright and license files remain authoritative for those files.

## Practical boundary

When modifying a project-owned MPL-covered file, keep its license notice intact. When importing or updating third-party code, preserve that code's upstream notices instead of stamping MPL over it.

This repository documentation describes project policy; it is not a substitute for reading the actual MPL-2.0 text when redistribution obligations matter.

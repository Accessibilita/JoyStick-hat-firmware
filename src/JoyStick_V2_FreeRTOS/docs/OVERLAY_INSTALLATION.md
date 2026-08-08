# Overlay Installation

Two ZIP layouts are distributed from the same Phase-1 project.

## Repository overlay

Use `JoyStick_Interface_Firmware_Phase1_REPO_OVERLAY.zip` from the root of the
`Accessibilita/JoyStick-Interface-firmware` checkout. It contains the path:

```text
src/JoyStick_V2_FreeRTOS/
```

Recommended sequence:

```bash
git switch -c phase1-safety-foundation
unzip -o JoyStick_Interface_Firmware_Phase1_REPO_OVERLAY.zip
src/JoyStick_V2_FreeRTOS/tools/cleanup_legacy_overlay.sh
src/JoyStick_V2_FreeRTOS/tools/ide_preflight.sh
```

The cleanup script removes only known obsolete files from the historical Cube
project. It does not touch files elsewhere in the repository.

If the project is already open in STM32CubeIDE, right-click it and select
**Refresh**, then run **Project -> Clean**. The Eclipse project name remains
`JoyStick_V2_FreeRTOS`, so a second project is not created.

## Workspace extraction

Use `JoyStick_V2_FreeRTOS_Phase1_WORKSPACE.zip` from the root of the desired
STM32CubeIDE workspace. It contains:

```text
JoyStick_V2_FreeRTOS/
```

Then import it with:

```text
File -> Import -> General -> Existing Projects into Workspace
```

Do not enable **Copy projects into workspace** when the project has already
been extracted there.

## Important

Do not extract both packages into the same location. They contain the same
project with different top-level paths for two different installation targets.

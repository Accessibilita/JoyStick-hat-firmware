# STM32CubeIDE compile and JTAG/SWD workflow

## 1. Install/import the STM32CubeF4 package

Use STM32CubeIDE's embedded software package manager to install STM32CubeF4 V1.28.1, or place
that package under:

```text
~/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1
```

From a terminal in the project root:

```bash
tools/bootstrap_cube_dependencies.sh
make host-test
```

The bootstrap copies CMSIS, the STM32F4 HAL, and FreeRTOS into the conventional `Drivers/` and
`Middlewares/` folders. The IDE and command-line Makefile therefore compile the same files.

## 2. Import

In STM32CubeIDE:

1. **File → Import → General → Existing Projects into Workspace**.
2. Select the project root containing `.project` and `.cproject`.
3. Do not select **Copy projects into workspace** unless you intentionally want another copy.
4. Select the **Debug** build configuration.
5. Build the project.

The Debug configuration calls the project-owned target `make debug`; Release calls
`make release`. STM32CubeIDE remains the editor, indexer, build console, ELF debugger, register
viewer, and peripheral viewer without maintaining a second generated build description.

## 3. Debug configuration

Create **Run → Debug Configurations → STM32 C/C++ Application** and select:

```text
build/Debug/JoyStick_V2_FreeRTOS.elf
```

Select the STM32F446VET6 and the connected ST-LINK/JTAG probe. Use **connect under reset** for
initial board bring-up. SWD is generally sufficient and leaves more pins available, but the
firmware does not disable the reset-default SWJ interface.

Recommended first breakpoints:

```text
main
MX_GPIO_Init
MX_ADC1_Init
InputAcquisition_OnDmaHalfCompleteFromIsr
InputAcquisition_CopyCompletedBatch
InputDiagnostics_Evaluate
SafetyState_Step
WatchdogSupervision_RefreshIfHealthy
Error_Handler
```

Recommended Expressions view symbol:

```text
g_app_phase1_debug_snapshot
```

A completed snapshot has an even `snapshot_sequence`; an odd value means the debugger observed
it during an update.

## 4. First hardware checks

Before connecting any motor-control cable:

1. Verify PC9 (`RS485_DE`) remains low from reset onward.
2. Verify PA6 and PC4 keep the LED drivers blanked.
3. Verify TIM2 update rate is 1 kHz.
4. Verify DMA2 Stream 0 alternates half/full callbacks every 5 ms.
5. Move the joystick and watch the two raw ADC counts.
6. Disconnect or fault each input only on a current-limited bench setup and confirm faults.
7. Halt for longer than two seconds and verify the Debug build does not reset from IWDG.
8. Run without halting and confirm the safety-loop counter advances.

## 5. CubeMX regeneration warning

The `.ioc` in this package is a reviewed migration manifest, not yet a proven regeneration
source. Do not press **Generate Code** over this tree during Phase 1 acceptance. First open it in
the installed CubeMX version, compare every pin and peripheral against `HARDWARE_PINMAP.md`, and
generate into a temporary staging directory. Merge only a reviewed diff.

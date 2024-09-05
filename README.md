# JoyStick-hat-firmware
Firmware for the JoyStick Hat Board 

# JoyStick-hat Firmware Compilation Guide with STM32CubeIDE

## 1. Prerequisites

Before starting, ensure the following are installed:

- **STM32CubeIDE** (download from [STMicroelectronics](https://www.st.com/en/development-tools/stm32cubeide.html)).
- **Git** (optional, if cloning the repository).

## 2. Clone the Repository

To download the firmware source code, clone the repository from GitHub using the following command:

```bash
git clone https://github.com/Accessibilita/JoyStick-hat-firmware.git
```

Alternatively, download the repository as a ZIP file and extract it.

## 3. Open the Project in STM32CubeIDE

- Launch **STM32CubeIDE**.
  
- Import the Project:
  - Go to `File -> Open Projects from File System`.
  - Select the root directory of the cloned or extracted project.
  - STM32CubeIDE should detect the project automatically. Click `Finish`.

## 4. Configure the Target MCU (Optional)

- In the project explorer, open the `.ioc` file.
- Verify that the correct target microcontroller is selected (e.g., STM32F4).
- Modify any peripheral or clock configurations if necessary.

## 5. Build the Project

- In the **Project Explorer**, right-click the project folder.
- Select `Build Project`.
- The build progress will appear in the console. A successful build will show a `BUILD SUCCESSFUL` message.

## 6. Flash the Firmware to the MCU

- Connect your STM32 Board to your PC via USB or ST-LINK programmer.

- Load the Firmware:
  - Click `Run -> Debug As -> STM32 Cortex-M C/C++ Application`.
  - STM32CubeIDE will compile (if necessary) and load the firmware onto the MCU.

- Start Debugging:
  - The debugger will start automatically once flashing is complete.
  - You can now monitor the execution or debug the firmware.

## 7. Troubleshooting

- **Build Issues**: Check that all required files and libraries are present. Review the `.ioc` file for any incorrect configurations.

- **Flashing Issues**: Ensure that ST-LINK drivers are correctly installed and that the STM32 board is detected by your system. Verify jumper settings on the board (e.g., Boot0 pin).

## 8. Additional Notes

- **Code Customization**: Modify source files as needed, regenerate code from STM32CubeMX (if the `.ioc` file is modified), and recompile the project.

- **Hardware Compatibility**: Ensure that the firmware and pin configurations match the STM32 board you are using.

# STM32CubeIDE Setup Guide (Windows, Linux, macOS)

This document explains how to install and set up **STM32CubeIDE** on Windows, Linux, and macOS. STM32CubeIDE is a free integrated development environment from STMicroelectronics for STM32 microcontrollers.

---

## 1. Overview

**STM32CubeIDE** includes:
- An IDE based on Eclipse™
- C/C++ toolchain for ARM® Cortex®-M
- STM32CubeMX for project initialization and code generation
- Debugging and programming tools

You can download STM32CubeIDE from the official [STMicroelectronics website](https://www.st.com/en/development-tools/stm32cubeide.html). An ST account is required to download. Once you have an account and are logged in, follow the OS-specific steps below.

---

## 2. System Requirements

- **Recommended RAM**: 4 GB or more
- **Disk Space**: ~1 GB for full installation
- **JRE/JDK**: Bundled with installer (no separate download usually needed)
- **Additional Tools**:
  - USB drivers (for Windows) to connect your STM32 device
  - `udev` rules (for Linux) to access debugger/programmer devices
  - Xcode command-line tools (for macOS, optional but recommended)

---

## 3. Windows Installation

1. **Download the Installer**  
   - Visit the [STM32CubeIDE page](https://www.st.com/en/development-tools/stm32cubeide.html).  
   - Click **Get Software** and choose **Windows** (x86_64).

2. **Run the Installer**  
   - Double-click the downloaded `.exe` file.  
   - If prompted by User Account Control, click **Yes**.

3. **Follow the On-Screen Instructions**  
   - Accept the license agreement.  
   - Select the installation folder (default is usually fine).  
   - Choose the components you want to install (debuggers, additional libraries, etc.).  
   - Complete the installation.

4. **Driver Installation** (Optional but recommended)  
   - If you have an ST-Link or other debug probe, install the appropriate drivers when prompted or from the ST website.  
   - Ensure your device is recognized correctly in the **Device Manager**.

5. **Launch STM32CubeIDE**  
   - After installation, go to **Start Menu** → **STMicroelectronics** → **STM32CubeIDE**.  
   - On first launch, select a workspace folder (where your projects will be saved).

---

## 4. Linux Installation

1. **Download the Installer**  
   - Go to the [STM32CubeIDE page](https://www.st.com/en/development-tools/stm32cubeide.html).  
   - Click **Get Software** and select **Linux** (x86_64).

2. **Make the Installer Executable**  
   - Open a terminal in the directory containing the downloaded `.sh` file.  
   - Run:  
     ```bash
     chmod +x ./SetupSTM32CubeIDE-<version>-linux.sh
     ```

3. **Run the Installer**  
   - From the terminal, run:  
     ```bash
     ./SetupSTM32CubeIDE-<version>-linux.sh
     ```  
   - Follow the on-screen prompts.  
   - Accept the license agreement, choose the installation path, etc.

4. **Set Up `udev` Rules** (Optional but recommended)  
   - If you plan to use ST-Link or other USB-based debuggers without `sudo`, create a new file in `/etc/udev/rules.d/` (for example, `99-stlink.rules`) with the appropriate permissions. Example:  
     ```bash
     sudo nano /etc/udev/rules.d/99-stlink.rules
     # Paste your rules here (refer to official ST documentation for correct rules)
     ```  
   - Then reload the rules:  
     ```bash
     sudo udevadm control --reload-rules && sudo udevadm trigger
     ```

5. **Launch STM32CubeIDE**  
   - After installation, you can launch it from your desktop environment or by running:  
     ```bash
     /path/to/stm32cubeide/stm32cubeide
     ```

---

## 5. macOS Installation

1. **Download the Installer**  
   - Visit the [STM32CubeIDE page](https://www.st.com/en/development-tools/stm32cubeide.html).  
   - Click **Get Software** and select **macOS** (x86_64 or Apple Silicon if available).

2. **Install Xcode Command-Line Tools** (Optional but recommended)  
   - Open **Terminal** and run:  
     ```bash
     xcode-select --install
     ```
   - Follow prompts to install.

3. **Open the Disk Image**  
   - Double-click the downloaded `.dmg` file to mount it.  
   - A new window will appear with the **STM32CubeIDE** application.

4. **Install STM32CubeIDE**  
   - Drag the **STM32CubeIDE** icon into the **Applications** folder.  

5. **Grant Permissions** (if needed)  
   - If macOS flags it as unverified developer software, go to **System Settings** → **Privacy & Security** and allow the app to open.

6. **Launch STM32CubeIDE**  
   - From the **Applications** folder, double-click **STM32CubeIDE**.  
   - On first launch, select your workspace location.

---

## 6. Verifying the Installation

1. **Open STM32CubeIDE**  
   - Confirm that the application launches without errors.

2. **Check Version**  
   - Go to **Help** → **About STM32CubeIDE** (or **STM32CubeIDE** → **About** on macOS) to see the version and build details.

3. **Test with a Sample Project**  
   - Go to **File** → **New** → **STM32 Project**.  
   - Choose a board or MCU, and follow the wizard.  
   - Generate code with **STM32CubeMX** and build the project.

4. **Debug and Run**  
   - Connect an STM32 board via USB (ensure drivers or udev rules are properly configured).  
   - Click the **Debug** button.  
   - Verify that you can load the program onto the device and that debugging functions correctly.

---

## 7. Tips & Best Practices

- **Keep STM32CubeIDE Updated**: Check regularly for updates via **Help** → **Check for Updates**.
- **Manage Multiple Toolchains**: If you have multiple ARM toolchains, ensure that the correct one is selected in **Project Properties** → **C/C++ Build**.
- **Enable Automatic Code Generation**: Use **STM32CubeMX** settings to automatically regenerate project files after making changes to your MCU configuration.
- **Documentation & Community Support**:
  - Official [STMicroelectronics Forum](https://community.st.com/s/)
  - [STM32CubeIDE User Guide](https://www.st.com/resource/en/user_manual/dm00501007.pdf)

---

## 8. Conclusion

You have successfully installed **STM32CubeIDE** on your preferred operating system. From here, you can start creating and debugging STM32 projects with ease. Refer to the official documentation and community forums for more in-depth tutorials and troubleshooting.

**Happy coding with STM32!**

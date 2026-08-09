PROJECT := JoyStick_V2_FreeRTOS
BUILD ?= Debug
BUILD_DIR := build/$(BUILD)

# ---------------------------------------------------------------------------
# STM32CubeIDE toolchain baseline
#
# Phase 1 is developed and validated against STM32CubeIDE 2.2.0.
#
# The project deliberately uses the GNU Arm toolchain installed with that IDE
# when it is available.  This keeps terminal builds and CubeIDE builds on the
# same compiler instead of silently selecting an older CubeIDE installation.
#
# TOOLCHAIN_PREFIX may still be supplied explicitly by CI or a developer when
# intentional toolchain override testing is required.
# ---------------------------------------------------------------------------

CUBEIDE_VERSION ?= 2.2.0

CUBEIDE_ROOT_CANDIDATES := \
  /opt/st/stm32cubeide_$(CUBEIDE_VERSION) \
  /opt/st/STM32CubeIDE_$(CUBEIDE_VERSION) \
  $(HOME)/ST/stm32cubeide_$(CUBEIDE_VERSION) \
  $(HOME)/ST/STM32CubeIDE_$(CUBEIDE_VERSION)

# More than one GNU Tools plug-in can exist inside an IDE installation.
# Select the newest one belonging specifically to the requested CubeIDE
# installation.  Do NOT search older CubeIDE installations here.
CUBEIDE_ARM_GCC ?= $(shell \
  for root in $(CUBEIDE_ROOT_CANDIDATES); do \
    if [ -d "$$root/plugins" ]; then \
      find "$$root/plugins" \
        -type f \
        -path '*/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin/arm-none-eabi-gcc' \
        2>/dev/null; \
    fi; \
  done | sort -V | tail -n 1)

ifeq ($(origin TOOLCHAIN_PREFIX),undefined)
ifneq ($(strip $(CUBEIDE_ARM_GCC)),)
TOOLCHAIN_PREFIX := $(patsubst %gcc,%,$(CUBEIDE_ARM_GCC))
else ifneq ($(shell command -v arm-none-eabi-gcc 2>/dev/null),)
# Fallback for CI or development hosts without STM32CubeIDE installed.
TOOLCHAIN_PREFIX := arm-none-eabi-
else
TOOLCHAIN_PREFIX := arm-none-eabi-
endif
endif

CC := $(TOOLCHAIN_PREFIX)gcc
AS := $(TOOLCHAIN_PREFIX)gcc
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
SIZE := $(TOOLCHAIN_PREFIX)size

# Host-only goals must not require an installed STM32 package or ARM compiler.
NON_TARGET_GOALS := host-test host-sanitize host-analyze phase1-check phase2-check phase3-check phase4-check clean help
ifeq ($(strip $(MAKECMDGOALS)),)
NEED_TARGET_DEPS := 1
else ifneq ($(strip $(filter-out $(NON_TARGET_GOALS),$(MAKECMDGOALS))),)
NEED_TARGET_DEPS := 1
else
NEED_TARGET_DEPS := 0
endif

STM32CUBE_F4_PATH ?= $(firstword \
  $(wildcard $(HOME)/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1) \
  $(wildcard $(HOME)/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.*))

# Prefer the normal CubeIDE project-local dependency layout created by the
# bootstrap script. Both CubeIDE and this Makefile then see the same sources.
ifneq ($(wildcard Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c),)
CUBE_ROOT := .
else
CUBE_ROOT := $(STM32CUBE_F4_PATH)
endif

ifeq ($(NEED_TARGET_DEPS),1)
ifeq ($(strip $(CUBE_ROOT)),)
$(error STM32CubeF4 not found. Run tools/bootstrap_cube_dependencies.sh or set STM32CUBE_F4_PATH)
endif
endif

CPU_FLAGS := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
BASE_C_FLAGS := $(CPU_FLAGS) -std=c11 -ffunction-sections -fdata-sections \
  -Wall -Wextra -Wshadow -Wformat=2 -Wundef -Wno-unused-parameter
PROJECT_WARNING_FLAGS := -Werror -Wconversion -Wdouble-promotion \
  -Wstrict-prototypes -Wmissing-prototypes
VENDOR_WARNING_FLAGS := -Wno-sign-conversion -Wno-conversion \
  -Wno-missing-prototypes -Wno-strict-prototypes

ifeq ($(BUILD),Release)
OPT_FLAGS := -O2 -g1 -DNDEBUG
else
OPT_FLAGS := -Og -g3 -DDEBUG
endif

DEFINES := -DUSE_HAL_DRIVER -DSTM32F446xx
INCLUDES := \
  -ICore/Inc \
  -IApp/Inc \
  -IPlatform/Inc \
  -I$(CUBE_ROOT)/Drivers/STM32F4xx_HAL_Driver/Inc \
  -I$(CUBE_ROOT)/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy \
  -I$(CUBE_ROOT)/Drivers/CMSIS/Device/ST/STM32F4xx/Include \
  -I$(CUBE_ROOT)/Drivers/CMSIS/Include \
  -I$(CUBE_ROOT)/Middlewares/Third_Party/FreeRTOS/Source/include \
  -I$(CUBE_ROOT)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F

PROJECT_SOURCES := \
  $(wildcard App/Src/*.c) \
  $(wildcard Platform/Src/*.c) \
  Core/Src/main.c \
  Core/Src/gpio.c \
  Core/Src/dma.c \
  Core/Src/adc.c \
  Core/Src/spi.c \
  Core/Src/tim.c \
  Core/Src/iwdg.c \
  Core/Src/stm32f4xx_hal_msp.c \
  Core/Src/stm32f4xx_it.c

HAL_NAMES := \
  stm32f4xx_hal \
  stm32f4xx_hal_adc \
  stm32f4xx_hal_adc_ex \
  stm32f4xx_hal_cortex \
  stm32f4xx_hal_dma \
  stm32f4xx_hal_dma_ex \
  stm32f4xx_hal_flash \
  stm32f4xx_hal_flash_ex \
  stm32f4xx_hal_gpio \
  stm32f4xx_hal_iwdg \
  stm32f4xx_hal_pwr \
  stm32f4xx_hal_pwr_ex \
  stm32f4xx_hal_rcc \
  stm32f4xx_hal_rcc_ex \
  stm32f4xx_hal_spi \
  stm32f4xx_hal_tim \
  stm32f4xx_hal_tim_ex
HAL_SOURCES := $(addprefix $(CUBE_ROOT)/Drivers/STM32F4xx_HAL_Driver/Src/,$(addsuffix .c,$(HAL_NAMES)))

FREERTOS_ROOT := $(CUBE_ROOT)/Middlewares/Third_Party/FreeRTOS/Source
FREERTOS_NAMES := tasks queue list
FREERTOS_SOURCES := $(addprefix $(FREERTOS_ROOT)/,$(addsuffix .c,$(FREERTOS_NAMES))) \
  $(FREERTOS_ROOT)/portable/GCC/ARM_CM4F/port.c

CMSIS_SYSTEM := $(CUBE_ROOT)/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c
STARTUP := $(CUBE_ROOT)/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f446xx.s

PROJECT_OBJECTS := $(addprefix $(BUILD_DIR)/,$(PROJECT_SOURCES:.c=.o))
HAL_OBJECTS := $(addprefix $(BUILD_DIR)/Vendor/HAL/,$(addsuffix .o,$(HAL_NAMES)))
FREERTOS_OBJECTS := $(addprefix $(BUILD_DIR)/Vendor/FreeRTOS/,$(addsuffix .o,$(FREERTOS_NAMES))) \
  $(BUILD_DIR)/Vendor/FreeRTOS/port.o
CMSIS_OBJECT := $(BUILD_DIR)/Vendor/CMSIS/system_stm32f4xx.o
STARTUP_OBJECT := $(BUILD_DIR)/Vendor/CMSIS/startup_stm32f446xx.o
OBJECTS := $(PROJECT_OBJECTS) $(HAL_OBJECTS) $(FREERTOS_OBJECTS) $(CMSIS_OBJECT) $(STARTUP_OBJECT)
DEPS := $(OBJECTS:.o=.d)

PROJECT_CFLAGS := $(BASE_C_FLAGS) $(PROJECT_WARNING_FLAGS) $(OPT_FLAGS) $(DEFINES) $(INCLUDES) -MMD -MP
VENDOR_CFLAGS := $(BASE_C_FLAGS) $(VENDOR_WARNING_FLAGS) $(OPT_FLAGS) $(DEFINES) $(INCLUDES) -MMD -MP
ASFLAGS := $(CPU_FLAGS) $(OPT_FLAGS) -x assembler-with-cpp $(DEFINES) $(INCLUDES)
LDFLAGS := $(CPU_FLAGS) -TSTM32F446VETX_FLASH.ld \
  --specs=nano.specs --specs=nosys.specs \
  -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map \
  -Wl,--print-memory-usage -static
LDLIBS := -Wl,--start-group -lc -lm -lgcc -Wl,--end-group

.PHONY: all debug release target clean size flash host-test host-sanitize host-analyze phase1-check phase2-check phase3-check phase4-check check-tools help
all: debug

debug:
	$(MAKE) BUILD=Debug target

release:
	$(MAKE) BUILD=Release target

target: $(BUILD_DIR)/$(PROJECT).elf

$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
	$(OBJCOPY) -O ihex $@ $(BUILD_DIR)/$(PROJECT).hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/$(PROJECT).bin
	$(SIZE) $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(PROJECT_CFLAGS) -c $< -o $@

$(BUILD_DIR)/Vendor/HAL/%.o: $(CUBE_ROOT)/Drivers/STM32F4xx_HAL_Driver/Src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(VENDOR_CFLAGS) -c $< -o $@

$(BUILD_DIR)/Vendor/FreeRTOS/%.o: $(FREERTOS_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(VENDOR_CFLAGS) -c $< -o $@

$(BUILD_DIR)/Vendor/FreeRTOS/port.o: $(FREERTOS_ROOT)/portable/GCC/ARM_CM4F/port.c
	@mkdir -p $(dir $@)
	$(CC) $(VENDOR_CFLAGS) -c $< -o $@

$(CMSIS_OBJECT): $(CMSIS_SYSTEM)
	@mkdir -p $(dir $@)
	$(CC) $(VENDOR_CFLAGS) -c $< -o $@

$(STARTUP_OBJECT): $(STARTUP)
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

check-tools:
	@command -v $(CC) >/dev/null || { echo "Missing $(CC)"; exit 1; }
	@command -v $(OBJCOPY) >/dev/null || { echo "Missing $(OBJCOPY)"; exit 1; }

size:
	$(SIZE) $(BUILD_DIR)/$(PROJECT).elf

flash:
	STM32_Programmer_CLI -c port=SWD -w $(BUILD_DIR)/$(PROJECT).elf -v -rst

phase1-check:
	python3 tools/check_phase1_invariants.py

phase2-check: phase1-check
	python3 tools/check_phase2_invariants.py

phase3-check: phase2-check
	python3 tools/check_phase3_invariants.py

phase4-check: phase3-check
	python3 tools/check_phase4_invariants.py

host-test: phase4-check
	$(MAKE) -C tests/host test

host-sanitize: phase4-check
	$(MAKE) -C tests/host sanitize

host-analyze: phase4-check
	$(MAKE) -C tests/host analyze

clean:
	rm -rf build
	$(MAKE) -C tests/host clean

help:
	@printf '%s\n' \
	  'make host-test     Run Phase-1 + Phase-2 + Phase-3 + Phase-4 portable host tests' \
	  'make host-sanitize Run all host tests under ASan + UBSan' \
	  'make host-analyze  Run GCC -fanalyzer on Phase-2 + Phase-3 + Phase-4 host code' \
	  'make phase1-check  Verify original non-drive Phase-1 invariants' \
	  'make phase2-check  Verify Phase-2 protocol + physical-inhibit invariants' \
	  'make phase3-check  Verify Phase-3 config/shaping + physical-inhibit invariants' \
	  'make phase4-check  Verify Phase-4 runtime/HMI + physical-inhibit invariants' \
	  'make debug         Build target Debug ELF/HEX/BIN' \
	  'make release      Build target Release ELF/HEX/BIN' \
	  'make flash        Flash Debug ELF through STM32CubeProgrammer'

-include $(DEPS)

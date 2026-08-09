# SPDX-License-Identifier: MPL-2.0
#
# Accessibilita JoyStick Interface Firmware
# Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
# informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.

CC ?= cc
ANALYZER_CC ?= gcc
BASE_CFLAGS := -std=c11 -O0 -g3 -Wall -Wextra -Werror -Wshadow \
               -Wconversion -Wformat=2 -Wstrict-prototypes \
               -Wmissing-prototypes
INCLUDES := -I../../App/Inc -I.

PHASE1_SOURCES := test_phase1.c \
                  ../../App/Src/input_diagnostics.c \
                  ../../App/Src/safety_state.c \
                  ../../App/Src/health_monitor.c
PHASE1_TARGET := phase1_host_tests

PHASE2_SOURCES := test_phase2.c \
                  fake_motor_controller.c \
                  ../../App/Src/motor_protocol.c \
                  ../../App/Src/motor_link_state.c \
                  ../../App/Src/command_authorization.c
PHASE2_TARGET := phase2_host_tests
PHASE2_ANALYZER_TARGET := phase2_analyzer_tests

PHASE3_SOURCES := test_phase3.c \
                  ../../App/Src/joystick_configuration.c \
                  ../../App/Src/joystick_calibration.c \
                  ../../App/Src/joystick_processing.c
PHASE3_TARGET := phase3_host_tests
PHASE3_ANALYZER_TARGET := phase3_analyzer_tests

PHASE4_SOURCES := test_phase4.c \
                  ../../App/Src/configuration_runtime.c \
                  ../../App/Src/hmi_model.c \
                  ../../App/Src/runtime_control.c \
                  ../../App/Src/operating_mode.c \
                  ../../App/Src/joystick_configuration.c \
                  ../../App/Src/joystick_processing.c \
                  ../../App/Src/safety_state.c \
                  ../../App/Src/command_authorization.c
PHASE4_TARGET := phase4_host_tests
PHASE4_ANALYZER_TARGET := phase4_analyzer_tests

PHASE5_SOURCES := test_phase5.c \
                  phase5_system_simulator.c \
                  fake_motor_controller.c \
                  ../../App/Src/input_diagnostics.c \
                  ../../App/Src/configuration_runtime.c \
                  ../../App/Src/joystick_configuration.c \
                  ../../App/Src/joystick_processing.c \
                  ../../App/Src/runtime_control.c \
                  ../../App/Src/operating_mode.c \
                  ../../App/Src/safety_state.c \
                  ../../App/Src/command_authorization.c \
                  ../../App/Src/motor_protocol.c \
                  ../../App/Src/motor_link_state.c
PHASE5_TARGET := phase5_host_tests
PHASE5_ANALYZER_TARGET := phase5_analyzer_tests

.PHONY: test sanitize analyze clean

test: CFLAGS := $(BASE_CFLAGS)
test: $(PHASE1_TARGET) $(PHASE2_TARGET) $(PHASE3_TARGET) $(PHASE4_TARGET) $(PHASE5_TARGET)
	./$(PHASE1_TARGET)
	./$(PHASE2_TARGET)
	./$(PHASE3_TARGET)
	./$(PHASE4_TARGET)
	./$(PHASE5_TARGET)

sanitize: CFLAGS := $(BASE_CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer
sanitize: clean $(PHASE1_TARGET) $(PHASE2_TARGET) $(PHASE3_TARGET) $(PHASE4_TARGET) $(PHASE5_TARGET)
	./$(PHASE1_TARGET)
	./$(PHASE2_TARGET)
	./$(PHASE3_TARGET)
	./$(PHASE4_TARGET)
	./$(PHASE5_TARGET)

analyze:
	@command -v $(ANALYZER_CC) >/dev/null || { echo "Missing $(ANALYZER_CC) for -fanalyzer"; exit 1; }
	$(ANALYZER_CC) $(BASE_CFLAGS) -fanalyzer $(INCLUDES) $(PHASE2_SOURCES) -o $(PHASE2_ANALYZER_TARGET)
	$(ANALYZER_CC) $(BASE_CFLAGS) -fanalyzer $(INCLUDES) $(PHASE3_SOURCES) -o $(PHASE3_ANALYZER_TARGET)
	$(ANALYZER_CC) $(BASE_CFLAGS) -fanalyzer $(INCLUDES) $(PHASE4_SOURCES) -o $(PHASE4_ANALYZER_TARGET)
	$(ANALYZER_CC) $(BASE_CFLAGS) -fanalyzer $(INCLUDES) $(PHASE5_SOURCES) -o $(PHASE5_ANALYZER_TARGET)
	rm -f $(PHASE2_ANALYZER_TARGET) $(PHASE3_ANALYZER_TARGET) $(PHASE4_ANALYZER_TARGET) $(PHASE5_ANALYZER_TARGET)

$(PHASE1_TARGET): $(PHASE1_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE1_SOURCES) -o $@

$(PHASE2_TARGET): $(PHASE2_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE2_SOURCES) -o $@

$(PHASE3_TARGET): $(PHASE3_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE3_SOURCES) -o $@

$(PHASE4_TARGET): $(PHASE4_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE4_SOURCES) -o $@

$(PHASE5_TARGET): $(PHASE5_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE5_SOURCES) -o $@

clean:
	rm -f $(PHASE1_TARGET) $(PHASE2_TARGET) $(PHASE3_TARGET) $(PHASE4_TARGET) $(PHASE5_TARGET) \
	      $(PHASE2_ANALYZER_TARGET) $(PHASE3_ANALYZER_TARGET) $(PHASE4_ANALYZER_TARGET) $(PHASE5_ANALYZER_TARGET)

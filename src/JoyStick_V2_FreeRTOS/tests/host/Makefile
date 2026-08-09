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
ANALYZER_TARGET := phase2_analyzer_tests

.PHONY: test sanitize analyze clean

test: CFLAGS := $(BASE_CFLAGS)
test: $(PHASE1_TARGET) $(PHASE2_TARGET)
	./$(PHASE1_TARGET)
	./$(PHASE2_TARGET)

sanitize: CFLAGS := $(BASE_CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer
sanitize: clean $(PHASE1_TARGET) $(PHASE2_TARGET)
	./$(PHASE1_TARGET)
	./$(PHASE2_TARGET)

analyze:
	@command -v $(ANALYZER_CC) >/dev/null || { echo "Missing $(ANALYZER_CC) for -fanalyzer"; exit 1; }
	$(ANALYZER_CC) $(BASE_CFLAGS) -fanalyzer $(INCLUDES) $(PHASE2_SOURCES) -o $(ANALYZER_TARGET)
	rm -f $(ANALYZER_TARGET)

$(PHASE1_TARGET): $(PHASE1_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE1_SOURCES) -o $@

$(PHASE2_TARGET): $(PHASE2_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE2_SOURCES) -o $@

clean:
	rm -f $(PHASE1_TARGET) $(PHASE2_TARGET) $(ANALYZER_TARGET)

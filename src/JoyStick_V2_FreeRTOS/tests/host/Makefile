CC ?= cc
BASE_CFLAGS := -std=c11 -O0 -g3 -Wall -Wextra -Werror -Wshadow \
               -Wconversion -Wformat=2 -Wstrict-prototypes \
               -Wmissing-prototypes
INCLUDES := -I../../App/Inc
SOURCES := test_phase1.c \
           ../../App/Src/input_diagnostics.c \
           ../../App/Src/safety_state.c \
           ../../App/Src/health_monitor.c
TARGET := phase1_host_tests

.PHONY: test sanitize clean

test: CFLAGS := $(BASE_CFLAGS)
test: $(TARGET)
	./$(TARGET)

sanitize: CFLAGS := $(BASE_CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer
sanitize: clean $(TARGET)
	./$(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $@

clean:
	rm -f $(TARGET)

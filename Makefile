CC = gcc
CFLAGS = -Wall -Wextra -std=c99

BUILD_DIR = build
TARGET = $(BUILD_DIR)/cpu

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $@

$(TARGET): cpu.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean

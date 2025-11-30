# --- Directories ---
SRC_DIR := src
TEST_DIR := test
UNITY_DIR := lib/Unity/src
UNITY_FIXTURE_DIR := lib/Unity/extras/fixture/src
UNITY_MEMORY_DIR := lib/Unity/extras/memory/src

# --- Source files ---
SRC := $(wildcard $(SRC_DIR)/*.c)
TEST_SRC := $(wildcard $(TEST_DIR)/*.c)
UNITY_SRC := $(UNITY_DIR)/unity.c
UNITY_FIXTURE_SRC := $(UNITY_FIXTURE_DIR)/unity_fixture.c
UNITY_MEMORY_SRC := $(UNITY_MEMORY_DIR)/unity_memory.c

# --- Build output ---
BUILD_DIR := build
TARGET := $(BUILD_DIR)/tests

# --- Compiler settings ---
CC := gcc
CFLAGS := -I$(SRC_DIR) -I$(UNITY_DIR) -I$(UNITY_FIXTURE_DIR) -I$(UNITY_MEMORY_DIR) -Wall -Wextra -g

# --- Default target ---
all: test

# --- Build the test executable ---
$(TARGET): $(SRC) $(TEST_SRC) $(UNITY_SRC) $(UNITY_FIXTURE_SRC) $(UNITY_MEMORY_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# --- Run tests ---
test: $(TARGET)
	./$(TARGET)

# --- Create build directory if needed ---
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# --- Cleanup ---
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test clean
# Compiler and flags
CC := gcc
CFLAGS := -O3 -march=native -Wall -Wextra

# Directories
SRC_DIR := src
BUILD_DIR := build

# Name of the final executable
TARGET := carpathian

# Find all .c files in the src directory
SRCS := $(wildcard $(SRC_DIR)/*.c)

# Map the .c filenames to .o filenames in the build directory
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Default target
.PHONY: all
all: $(TARGET)

# Link the final binary
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile .c files into .o files
# The '| $(BUILD_DIR)' ensures the build directory exists before compiling
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create the build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean up build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
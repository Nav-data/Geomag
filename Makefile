# Makefile for GeoMag C Library
# World Magnetic Model (WMM) Implementation

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c11 -I./include
LDFLAGS = -lm

# Directories
SRC_DIR = src
INC_DIR = include
EXAMPLE_DIR = examples
TEST_DIR = test
BENCHMARK_DIR = benchmark
BUILD_DIR = build
DATA_DIR = data

# Source files
SRCS = $(SRC_DIR)/geomag.c
OBJS = $(BUILD_DIR)/geomag.o

# Targets
LIBRARY = $(BUILD_DIR)/libgeomag.a
SHARED_LIBRARY = $(BUILD_DIR)/libgeomag.so
EXAMPLE = $(BUILD_DIR)/example
TEST = $(BUILD_DIR)/test_geomag
BENCHMARK = $(BUILD_DIR)/benchmark

# Platform-specific shared library settings
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    SHARED_LIBRARY = $(BUILD_DIR)/libgeomag.dylib
    SHARED_FLAGS = -dynamiclib
else
    SHARED_LIBRARY = $(BUILD_DIR)/libgeomag.so
    SHARED_FLAGS = -shared
endif

# Default target
.PHONY: all
all: $(LIBRARY) $(SHARED_LIBRARY) $(EXAMPLE)

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/geomag.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# Build static library
$(LIBRARY): $(OBJS)
	@echo "Creating static library: $@"
	ar rcs $@ $^

# Build shared library
$(SHARED_LIBRARY): $(OBJS)
	@echo "Creating shared library: $@"
	$(CC) $(SHARED_FLAGS) -o $@ $^ $(LDFLAGS)

# Build example program
$(EXAMPLE): $(EXAMPLE_DIR)/example.c $(LIBRARY)
	@echo "Building example: $@"
	$(CC) $(CFLAGS) $< -L$(BUILD_DIR) -lgeomag $(LDFLAGS) -o $@

# Build test program
$(TEST): $(TEST_DIR)/test_geomag.c $(LIBRARY)
	@echo "Building test: $@"
	$(CC) $(CFLAGS) $< -L$(BUILD_DIR) -lgeomag $(LDFLAGS) -o $@

# Build benchmark program
$(BENCHMARK): $(BENCHMARK_DIR)/benchmark.c $(LIBRARY)
	@echo "Building benchmark: $@"
	$(CC) $(CFLAGS) $< -L$(BUILD_DIR) -lgeomag $(LDFLAGS) -o $@

# Run example
.PHONY: run-example
run-example: $(EXAMPLE)
	@echo "Running example..."
	@./$(EXAMPLE)

# Run tests
.PHONY: test
test: $(TEST)
	@echo "Running tests..."
	@./$(TEST)

# Run benchmark
.PHONY: benchmark
benchmark: $(BENCHMARK)
	@echo "Running benchmark..."
	@./$(BENCHMARK)

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)

# Install (optional - copies library and headers to system directories)
.PHONY: install
install: $(LIBRARY)
	@echo "Installing library and headers..."
	install -d /usr/local/lib
	install -d /usr/local/include
	install -m 644 $(LIBRARY) /usr/local/lib/
	install -m 644 $(INC_DIR)/geomag.h /usr/local/include/
	@echo "Installation complete!"

# Uninstall
.PHONY: uninstall
uninstall:
	@echo "Uninstalling library and headers..."
	rm -f /usr/local/lib/libgeomag.a
	rm -f /usr/local/include/geomag.h
	@echo "Uninstallation complete!"

# Help
.PHONY: help
help:
	@echo "GeoMag C Library - Makefile targets:"
	@echo "  all          - Build library and example (default)"
	@echo "  clean        - Remove build artifacts"
	@echo "  run-example  - Build and run the example program"
	@echo "  test         - Build and run tests"
	@echo "  benchmark    - Build and run performance benchmarks"
	@echo "  install      - Install library to /usr/local (requires sudo)"
	@echo "  uninstall    - Uninstall library from /usr/local (requires sudo)"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Build with optimizations:"
	@echo "  make CFLAGS='-O3 -march=native -I./include'"
	@echo ""
	@echo "Build with debug info:"
	@echo "  make CFLAGS='-g -O0 -I./include'"
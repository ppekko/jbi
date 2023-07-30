# Compiler and flags
CC := tcc
CFLAGS := -std=c99 -Wall -Wextra
LDFLAGS :=

# Directories
SRC_DIR := src/sources
HDR_DIR := src/headers
BUILD_DIR := build
TARGET := jbi
INSTALL_DIR := /usr/local/bin

# Source and header files
SRCS := $(wildcard $(SRC_DIR)/*.c)
HDRS := $(wildcard $(HDR_DIR)/*.h)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Linking the objects to generate the executable
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compiling C source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HDRS)
	$(CC) $(CFLAGS) -I$(HDR_DIR) -c -o $@ $<

# Install the binary to the system directory
install: $(TARGET)
	install -m 755 $(TARGET) $(INSTALL_DIR)

# Uninstall the binary from the system directory
uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)

# Clean the build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Phony targets
.PHONY: all clean install uninstall

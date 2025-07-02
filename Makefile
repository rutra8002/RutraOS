# Makefile for OS Boot Sector and Kernel

# Directories
SRC_DIR = src
BUILD_DIR = build

# Files
BOOT_ASM = $(SRC_DIR)/boot.asm
KERNEL_ASM = $(SRC_DIR)/kernel.asm
BOOT_BIN = $(BUILD_DIR)/boot.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
OS_IMAGE = $(BUILD_DIR)/os.img

# Tools
ASM = nasm
QEMU = qemu-system-x86_64

# Assembly flags
ASM_FLAGS = -f bin

# Default target
all: $(OS_IMAGE)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build boot sector
$(BOOT_BIN): $(BOOT_ASM) | $(BUILD_DIR)
	$(ASM) $(ASM_FLAGS) $< -o $@

# Build kernel
$(KERNEL_BIN): $(KERNEL_ASM) | $(BUILD_DIR)
	$(ASM) $(ASM_FLAGS) $< -o $@

# Create OS image by combining boot sector and kernel
$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN) | $(BUILD_DIR)
	cat $(BOOT_BIN) $(KERNEL_BIN) > $@

# Run in QEMU
run: $(OS_IMAGE)
	$(QEMU) $<

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Rebuild everything
rebuild: clean all

# Show help
help:
	@echo "Available targets:"
	@echo "  all     - Build the boot sector and kernel (default)"
	@echo "  run     - Build and run the OS image in QEMU"
	@echo "  clean   - Remove build artifacts"
	@echo "  rebuild - Clean and rebuild everything"
	@echo "  help    - Show this help message"

# Declare phony targets
.PHONY: all run clean rebuild help

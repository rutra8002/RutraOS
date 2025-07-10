# Makefile for OS with GRUB

# Directories
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = $(BUILD_DIR)/iso

# Source subdirectories
BOOT_DIR = $(SRC_DIR)/boot
KERNEL_DIR = $(SRC_DIR)/kernel
DRIVERS_DIR = $(SRC_DIR)/drivers
FS_DIR = $(SRC_DIR)/fs
LIB_DIR = $(SRC_DIR)/lib
INCLUDE_DIR = $(SRC_DIR)/include

# Files
BOOT_ASM = $(BOOT_DIR)/boot.asm
KERNEL_C_FILES = $(wildcard $(KERNEL_DIR)/*.c) $(wildcard $(DRIVERS_DIR)/*.c) $(wildcard $(FS_DIR)/*.c) $(wildcard $(LIB_DIR)/*.c)
BOOT_OBJ = $(BUILD_DIR)/boot.o
KERNEL_OBJS = $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/%.o,$(wildcard $(KERNEL_DIR)/*.c)) \
              $(patsubst $(DRIVERS_DIR)/%.c,$(BUILD_DIR)/%.o,$(wildcard $(DRIVERS_DIR)/*.c)) \
              $(patsubst $(FS_DIR)/%.c,$(BUILD_DIR)/%.o,$(wildcard $(FS_DIR)/*.c)) \
              $(patsubst $(LIB_DIR)/%.c,$(BUILD_DIR)/%.o,$(wildcard $(LIB_DIR)/*.c))
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
GRUB_CFG = grub.cfg
LINKER_SCRIPT = linker.ld
OS_ISO = $(BUILD_DIR)/os.iso

# Tools
ASM = nasm
CC = gcc
LD = ld
GRUB_MKRESCUE = grub-mkrescue
QEMU = qemu-system-x86_64

# Assembly flags
ASM_FLAGS = -f elf64

# C compiler flags
CC_FLAGS = -m64 -ffreestanding -fno-stack-protector -fno-builtin -nostdlib -nostdinc -Wall -Wextra -c -I$(INCLUDE_DIR)

# Linker flags
LD_FLAGS = -m elf_x86_64 -T $(LINKER_SCRIPT)

# Default target
all: $(OS_ISO)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create ISO directory structure
$(ISO_DIR): | $(BUILD_DIR)
	mkdir -p $(ISO_DIR)/boot/grub

# Build boot object file (assembly)
$(BOOT_OBJ): $(BOOT_ASM) | $(BUILD_DIR)
	$(ASM) $(ASM_FLAGS) $< -o $@

# Build kernel object files (C) - pattern rules for each directory
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(DRIVERS_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(FS_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) $< -o $@

# Link kernel binary
$(KERNEL_BIN): $(BOOT_OBJ) $(KERNEL_OBJS) $(LINKER_SCRIPT) | $(BUILD_DIR)
	$(LD) $(LD_FLAGS) $(BOOT_OBJ) $(KERNEL_OBJS) -o $@

# Create bootable ISO with GRUB
$(OS_ISO): $(KERNEL_BIN) $(GRUB_CFG) | $(ISO_DIR)
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	cp $(GRUB_CFG) $(ISO_DIR)/boot/grub/
	$(GRUB_MKRESCUE) -o $@ $(ISO_DIR)

# Run in QEMU
run: $(OS_ISO)
	$(QEMU) -cdrom $<

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

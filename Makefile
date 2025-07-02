# Makefile for OS with GRUB

# Directories
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = $(BUILD_DIR)/iso

# Files
KERNEL_ASM = $(SRC_DIR)/kernel.asm
KERNEL_OBJ = $(BUILD_DIR)/kernel.o
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
GRUB_CFG = grub.cfg
LINKER_SCRIPT = linker.ld
OS_ISO = $(BUILD_DIR)/os.iso

# Tools
ASM = nasm
LD = ld
GRUB_MKRESCUE = grub2-mkrescue
QEMU = qemu-system-x86_64

# Assembly flags
ASM_FLAGS = -f elf32

# Linker flags
LD_FLAGS = -m elf_i386 -T $(LINKER_SCRIPT)

# Default target
all: $(OS_ISO)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create ISO directory structure
$(ISO_DIR): | $(BUILD_DIR)
	mkdir -p $(ISO_DIR)/boot/grub

# Build kernel object file
$(KERNEL_OBJ): $(KERNEL_ASM) | $(BUILD_DIR)
	$(ASM) $(ASM_FLAGS) $< -o $@

# Link kernel binary
$(KERNEL_BIN): $(KERNEL_OBJ) $(LINKER_SCRIPT) | $(BUILD_DIR)
	$(LD) $(LD_FLAGS) $< -o $@

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

#include "ramdisk.h"
#include "fat12.h"
#include "memory.h"
#include "terminal.h"

// Global ramdisk instance
static ramdisk_t ramdisk;

int ramdisk_init(void) {
    if (ramdisk.initialized) {
        return 1; // Already initialized
    }
    
    // Allocate memory for the ramdisk
    ramdisk.data = (uint8_t*)kmalloc(RAMDISK_SIZE);
    if (!ramdisk.data) {
        terminal_writestring("ERROR: Failed to allocate ramdisk memory\n");
        return 0;
    }
    
    // Initialize ramdisk structure
    ramdisk.size = RAMDISK_SIZE;
    ramdisk.sector_size = RAMDISK_SECTOR_SIZE;
    ramdisk.sector_count = RAMDISK_SECTOR_COUNT;
    ramdisk.initialized = 1;
    
    // Clear the ramdisk
    memset(ramdisk.data, 0, RAMDISK_SIZE);
    
    terminal_writestring("Ramdisk initialized: ");
    char buffer[32];
    uint32_to_string(RAMDISK_SIZE, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes (");
    uint32_to_string(RAMDISK_SECTOR_COUNT, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" sectors)\n");
    
    return 1;
}

int ramdisk_read_sector(uint32_t sector, void* buffer) {
    if (!ramdisk.initialized) {
        return 0;
    }
    
    if (sector >= ramdisk.sector_count) {
        return 0; // Invalid sector
    }
    
    uint32_t offset = sector * ramdisk.sector_size;
    memcpy(buffer, ramdisk.data + offset, ramdisk.sector_size);
    return 1;
}

int ramdisk_write_sector(uint32_t sector, const void* buffer) {
    if (!ramdisk.initialized) {
        return 0;
    }
    
    if (sector >= ramdisk.sector_count) {
        return 0; // Invalid sector
    }
    
    uint32_t offset = sector * ramdisk.sector_size;
    memcpy(ramdisk.data + offset, buffer, ramdisk.sector_size);
    return 1;
}

int ramdisk_format_fat12(void) {
    if (!ramdisk.initialized) {
        terminal_writestring("ERROR: Ramdisk not initialized\n");
        return 0;
    }
    
    terminal_writestring("Formatting ramdisk with FAT12 filesystem...\n");
    
    // Format the ramdisk with FAT12
    if (!fat12_format()) {
        terminal_writestring("ERROR: FAT12 format failed\n");
        return 0;
    }
    
    terminal_writestring("FAT12 filesystem created successfully\n");
    
    // Create some demo files
    const char* readme_content = 
        "Welcome to RutraOS Ramdisk!\n"
        "This is a FAT12 filesystem running entirely in RAM.\n"
        ;
    
    if (fat12_create_file("README.TXT", readme_content, strlen(readme_content))) {
        terminal_writestring("Created README.TXT\n");
    }
    
    const char* demo_content = "This is a demo file in the ramdisk!";
    if (fat12_create_file("DEMO.TXT", demo_content, strlen(demo_content))) {
        terminal_writestring("Created DEMO.TXT\n");
    }
    
    return 1;
}

void ramdisk_print_info(void) {
    if (!ramdisk.initialized) {
        terminal_writestring("Ramdisk not initialized\n");
        return;
    }
    
    terminal_writestring("=== Ramdisk Information ===\n");
    
    char buffer[32];
    
    terminal_writestring("Size: ");
    uint32_to_string(ramdisk.size, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    terminal_writestring("Sector size: ");
    uint32_to_string(ramdisk.sector_size, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    terminal_writestring("Sector count: ");
    uint32_to_string(ramdisk.sector_count, buffer);
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    terminal_writestring("Memory address: 0x");
    uint32_to_hex((uint32_t)(uintptr_t)ramdisk.data, buffer);
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    terminal_writestring("Filesystem: FAT12\n");
}

ramdisk_t* get_ramdisk(void) {
    return &ramdisk;
}

// Helper function to calculate string length
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

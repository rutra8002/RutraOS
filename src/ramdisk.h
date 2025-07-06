#ifndef RAMDISK_H
#define RAMDISK_H

#include "kernel.h"

// Ramdisk configuration
#define RAMDISK_SIZE (512 * 1024)  // 512KB ramdisk
#define RAMDISK_SECTOR_SIZE 512
#define RAMDISK_SECTOR_COUNT (RAMDISK_SIZE / RAMDISK_SECTOR_SIZE)

// Ramdisk structure
typedef struct {
    uint8_t* data;
    size_t size;
    size_t sector_size;
    size_t sector_count;
    int initialized;
} ramdisk_t;

// Ramdisk functions
int ramdisk_init(void);
int ramdisk_read_sector(uint32_t sector, void* buffer);
int ramdisk_write_sector(uint32_t sector, const void* buffer);
int ramdisk_format_fat12(void);
void ramdisk_print_info(void);

// Get the ramdisk instance
ramdisk_t* get_ramdisk(void);

// Helper functions
size_t strlen(const char* str);

#endif // RAMDISK_H

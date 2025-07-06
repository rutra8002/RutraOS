#ifndef FAT12_H
#define FAT12_H

#include "kernel.h"

// FAT12 Constants
#define FAT12_SECTOR_SIZE 512
#define FAT12_CLUSTER_SIZE 1024  // 2 sectors per cluster
#define FAT12_ROOT_ENTRIES 224
#define FAT12_MEDIA_BYTE 0xF8   // Fixed disk
#define FAT12_SIGNATURE 0xAA55

// FAT12 Boot sector structure
typedef struct __attribute__((packed)) {
    uint8_t jump[3];                // Jump instruction
    uint8_t oem_name[8];           // OEM name
    uint16_t bytes_per_sector;      // Bytes per sector
    uint8_t sectors_per_cluster;    // Sectors per cluster
    uint16_t reserved_sectors;      // Reserved sectors
    uint8_t fat_count;             // Number of FATs
    uint16_t root_entries;         // Root directory entries
    uint16_t total_sectors;        // Total sectors (if < 65536)
    uint8_t media_descriptor;      // Media descriptor
    uint16_t sectors_per_fat;      // Sectors per FAT
    uint16_t sectors_per_track;    // Sectors per track
    uint16_t heads;                // Number of heads
    uint32_t hidden_sectors;       // Hidden sectors
    uint32_t total_sectors_32;     // Total sectors (if >= 65536)
    uint8_t drive_number;          // Drive number
    uint8_t reserved;              // Reserved
    uint8_t signature;             // Extended boot signature
    uint32_t volume_serial;        // Volume serial number
    uint8_t volume_label[11];      // Volume label
    uint8_t filesystem_type[8];    // File system type
    uint8_t boot_code[448];        // Boot code
    uint16_t boot_signature;       // Boot signature (0xAA55)
} fat12_boot_sector_t;

// FAT12 Directory entry structure
typedef struct __attribute__((packed)) {
    uint8_t name[8];               // File name (8 chars)
    uint8_t ext[3];                // Extension (3 chars)
    uint8_t attributes;            // File attributes
    uint8_t reserved;              // Reserved
    uint8_t create_time_tenth;     // Creation time (tenths of second)
    uint16_t create_time;          // Creation time
    uint16_t create_date;          // Creation date
    uint16_t access_date;          // Last access date
    uint16_t cluster_high;         // High 16 bits of cluster (always 0 for FAT12)
    uint16_t modify_time;          // Last modification time
    uint16_t modify_date;          // Last modification date
    uint16_t cluster_low;          // Low 16 bits of cluster
    uint32_t file_size;            // File size in bytes
} fat12_dir_entry_t;

// File attributes
#define FAT12_ATTR_READ_ONLY    0x01
#define FAT12_ATTR_HIDDEN       0x02
#define FAT12_ATTR_SYSTEM       0x04
#define FAT12_ATTR_VOLUME_LABEL 0x08
#define FAT12_ATTR_DIRECTORY    0x10
#define FAT12_ATTR_ARCHIVE      0x20
#define FAT12_ATTR_LONG_NAME    0x0F

// Special cluster values
#define FAT12_CLUSTER_FREE      0x000
#define FAT12_CLUSTER_RESERVED  0x001
#define FAT12_CLUSTER_BAD       0xFF7
#define FAT12_CLUSTER_END       0xFF8

// FAT12 filesystem functions
int fat12_format(void);
int fat12_create_file(const char* filename, const void* data, size_t size);
int fat12_read_file(const char* filename, void* buffer, size_t max_size);
int fat12_delete_file(const char* filename);
int fat12_list_files(void);
int fat12_get_file_size(const char* filename);

// Internal FAT12 functions
uint16_t fat12_get_fat_entry(uint16_t cluster);
void fat12_set_fat_entry(uint16_t cluster, uint16_t value);
uint16_t fat12_find_free_cluster(void);
int fat12_read_cluster(uint16_t cluster, void* buffer);
int fat12_write_cluster(uint16_t cluster, const void* buffer);

#endif // FAT12_H

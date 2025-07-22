#ifndef FAT16_H
#define FAT16_H

#include "kernel.h"

// FAT16 Constants
#define FAT16_SECTOR_SIZE 512
#define FAT16_CLUSTER_SIZE 1024  // 2 sectors per cluster
#define FAT16_ROOT_ENTRIES 512
#define FAT16_MEDIA_BYTE 0xF8   // Fixed disk
#define FAT16_SIGNATURE 0xAA55

// FAT16 Boot sector structure
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
} fat16_boot_sector_t;

// FAT16 Directory entry structure
typedef struct __attribute__((packed)) {
    uint8_t name[8];               // File name (8 chars)
    uint8_t ext[3];                // Extension (3 chars)
    uint8_t attributes;            // File attributes
    uint8_t reserved;              // Reserved
    uint8_t create_time_tenth;     // Creation time (tenths of second)
    uint16_t create_time;          // Creation time
    uint16_t create_date;          // Creation date
    uint16_t access_date;          // Last access date
    uint16_t cluster_high;         // High 16 bits of cluster (always 0 for FAT16)
    uint16_t modify_time;          // Last modification time
    uint16_t modify_date;          // Last modification date
    uint16_t cluster_low;          // Low 16 bits of cluster
    uint32_t file_size;            // File size in bytes
} fat16_dir_entry_t;

// File attributes
#define FAT16_ATTR_READ_ONLY    0x01
#define FAT16_ATTR_HIDDEN       0x02
#define FAT16_ATTR_SYSTEM       0x04
#define FAT16_ATTR_VOLUME_LABEL 0x08
#define FAT16_ATTR_DIRECTORY    0x10
#define FAT16_ATTR_ARCHIVE      0x20
#define FAT16_ATTR_LONG_NAME    0x0F

// Special cluster values
#define FAT16_CLUSTER_FREE      0x0000
#define FAT16_CLUSTER_RESERVED  0x0001
#define FAT16_CLUSTER_BAD       0xFFF7
#define FAT16_CLUSTER_END       0xFFF8

// FAT16 filesystem functions
int fat16_format(void);
int fat16_create_file(const char* filename, const void* data, size_t size);
int fat16_read_file(const char* filename, void* buffer, size_t max_size);
int fat16_delete_file(const char* filename);
int fat16_list_files(void);
int fat16_get_file_size(const char* filename);

// Directory functions
int fat16_create_directory(const char* dirname);
int fat16_change_directory(const char* path);
int fat16_list_directory(const char* path);
char* fat16_get_current_directory(void);
int fat16_is_directory(const char* name);

// Path functions
int fat16_parse_path(const char* path, char* components[], int max_components);
int fat16_find_directory_entry(const char* path, fat16_dir_entry_t* entry, uint16_t* parent_cluster);
uint16_t fat16_get_directory_cluster(const char* path);

// Internal FAT16 functions
uint16_t fat16_get_fat_entry(uint16_t cluster);
void fat16_set_fat_entry(uint16_t cluster, uint16_t value);
uint16_t fat16_find_free_cluster(void);
int fat16_read_cluster(uint16_t cluster, void* buffer);
int fat16_write_cluster(uint16_t cluster, const void* buffer);

#endif // FAT16_H

#include "fat16.h"
#include "ramdisk.h"
#include "memory.h"
#include "terminal.h"
#include "string.h"
#include "memory_utils.h"
#include "string.h"

// FAT16 layout in our ramdisk:
// Sector 0: Boot sector
// Sector 1-16: FAT1 (16 sectors for 512KB disk)
// Sector 17-32: FAT2 (backup)
// Sector 33-64: Root directory (32 sectors, 512 entries)
// Sector 65+: Data clusters

#define FAT16_BOOT_SECTOR     0
#define FAT16_FAT1_START      1
#define FAT16_FAT2_START      17
#define FAT16_ROOT_START      33
#define FAT16_DATA_START      65

#define FAT16_SECTORS_PER_FAT 16
#define FAT16_ROOT_SECTORS    32

// Helper function to convert string to uppercase FAT16 format
static void fat16_name_to_fatname(const char* name, char* fatname) {
    int i, j;
    
    // Initialize with spaces
    for (i = 0; i < 11; i++) {
        fatname[i] = ' ';
    }
    
    // Copy name part
    for (i = 0, j = 0; i < 8 && name[j] && name[j] != '.'; i++, j++) {
        if (name[j] >= 'a' && name[j] <= 'z') {
            fatname[i] = name[j] - 'a' + 'A';
        } else {
            fatname[i] = name[j];
        }
    }
    
    // Skip to extension
    while (name[j] && name[j] != '.') {
        j++;
    }
    
    // Copy extension
    if (name[j] == '.') {
        j++; // Skip dot
        for (i = 8; i < 11 && name[j]; i++, j++) {
            if (name[j] >= 'a' && name[j] <= 'z') {
                fatname[i] = name[j] - 'a' + 'A';
            } else {
                fatname[i] = name[j];
            }
        }
    }
}

// Helper function to compare FAT16 names
static int fat16_name_compare(const uint8_t* fatname1, const char* fatname2) {
    return memcmp(fatname1, fatname2, 11);
}

int fat16_format(void) {
    ramdisk_t* rd = get_ramdisk();
    if (!rd || !rd->initialized) {
        return 0;
    }
    
    // Create boot sector
    fat16_boot_sector_t boot_sector;
    memset(&boot_sector, 0, sizeof(boot_sector));
    
    // Jump instruction (3 bytes)
    boot_sector.jump[0] = 0xEB;
    boot_sector.jump[1] = 0x3C;
    boot_sector.jump[2] = 0x90;
    
    // OEM name
    memcpy(boot_sector.oem_name, "RutraOS ", 8);
    
    // BPB (BIOS Parameter Block)
    boot_sector.bytes_per_sector = FAT16_SECTOR_SIZE;
    boot_sector.sectors_per_cluster = 2;  // 1KB clusters
    boot_sector.reserved_sectors = 1;
    boot_sector.fat_count = 2;
    boot_sector.root_entries = FAT16_ROOT_ENTRIES;
    boot_sector.total_sectors = RAMDISK_SECTOR_COUNT;
    boot_sector.media_descriptor = FAT16_MEDIA_BYTE;
    boot_sector.sectors_per_fat = FAT16_SECTORS_PER_FAT;
    boot_sector.sectors_per_track = 18;  // Standard for 1.44MB floppy
    boot_sector.heads = 2;
    boot_sector.hidden_sectors = 0;
    boot_sector.total_sectors_32 = 0;
    
    // Extended BPB
    boot_sector.drive_number = 0x80;  // Hard disk
    boot_sector.reserved = 0;
    boot_sector.signature = 0x29;
    boot_sector.volume_serial = 0x12345678;
    memcpy(boot_sector.volume_label, "RUTRAOS    ", 11);
    memcpy(boot_sector.filesystem_type, "FAT16   ", 8);
    
    // Boot signature
    boot_sector.boot_signature = FAT16_SIGNATURE;
    
    // Write boot sector
    if (!ramdisk_write_sector(FAT16_BOOT_SECTOR, &boot_sector)) {
        return 0;
    }
    
    // Initialize FAT tables
    uint8_t fat_sector[FAT16_SECTOR_SIZE];
    memset(fat_sector, 0, FAT16_SECTOR_SIZE);
    
    // First FAT sector has special entries
    uint16_t* fat = (uint16_t*)fat_sector;
    fat[0] = 0xFF00 | FAT16_MEDIA_BYTE;  // Media descriptor
    fat[1] = 0xFFFF;                     // End of chain marker
    
    // Write first sector of both FATs
    ramdisk_write_sector(FAT16_FAT1_START, fat_sector);
    ramdisk_write_sector(FAT16_FAT2_START, fat_sector);
    
    // Clear remaining FAT sectors
    memset(fat_sector, 0, FAT16_SECTOR_SIZE);
    for (int i = 1; i < FAT16_SECTORS_PER_FAT; i++) {
        ramdisk_write_sector(FAT16_FAT1_START + i, fat_sector);
        ramdisk_write_sector(FAT16_FAT2_START + i, fat_sector);
    }
    
    // Clear root directory
    uint8_t root_sector[FAT16_SECTOR_SIZE];
    memset(root_sector, 0, FAT16_SECTOR_SIZE);
    for (int i = 0; i < FAT16_ROOT_SECTORS; i++) {
        ramdisk_write_sector(FAT16_ROOT_START + i, root_sector);
    }
    
    return 1;
}

uint16_t fat16_get_fat_entry(uint16_t cluster) {
    if (cluster < 2) {
        return 0;
    }
    
    // Calculate byte offset in FAT (FAT16 uses 2 bytes per entry)
    uint32_t byte_offset = cluster * 2;
    uint32_t sector = FAT16_FAT1_START + (byte_offset / FAT16_SECTOR_SIZE);
    uint32_t sector_offset = byte_offset % FAT16_SECTOR_SIZE;
    
    uint8_t fat_sector[FAT16_SECTOR_SIZE];
    if (!ramdisk_read_sector(sector, fat_sector)) {
        return 0;
    }
    
    // Read 16-bit value
    uint16_t* fat_entry = (uint16_t*)&fat_sector[sector_offset];
    return *fat_entry;
}

void fat16_set_fat_entry(uint16_t cluster, uint16_t value) {
    if (cluster < 2) {
        return;
    }
    
    // Calculate byte offset in FAT (FAT16 uses 2 bytes per entry)
    uint32_t byte_offset = cluster * 2;
    uint32_t sector = FAT16_FAT1_START + (byte_offset / FAT16_SECTOR_SIZE);
    uint32_t sector_offset = byte_offset % FAT16_SECTOR_SIZE;
    
    uint8_t fat_sector[FAT16_SECTOR_SIZE];
    if (!ramdisk_read_sector(sector, fat_sector)) {
        return;
    }
    
    // Write 16-bit value
    uint16_t* fat_entry = (uint16_t*)&fat_sector[sector_offset];
    *fat_entry = value;
    
    // Write updated sector to both FATs
    ramdisk_write_sector(sector, fat_sector);
    ramdisk_write_sector(sector + FAT16_SECTORS_PER_FAT, fat_sector);
}

uint16_t fat16_find_free_cluster(void) {
    // Start from cluster 2 (first data cluster)
    for (uint16_t cluster = 2; cluster < 65525; cluster++) {
        if (fat16_get_fat_entry(cluster) == FAT16_CLUSTER_FREE) {
            return cluster;
        }
    }
    return 0; // No free cluster found
}

int fat16_read_cluster(uint16_t cluster, void* buffer) {
    if (cluster < 2) {
        return 0;
    }
    
    // Calculate sector number
    uint32_t sector = FAT16_DATA_START + (cluster - 2) * 2;
    
    // Read both sectors of the cluster
    uint8_t* buf = (uint8_t*)buffer;
    if (!ramdisk_read_sector(sector, buf)) {
        return 0;
    }
    if (!ramdisk_read_sector(sector + 1, buf + FAT16_SECTOR_SIZE)) {
        return 0;
    }
    
    return 1;
}

int fat16_write_cluster(uint16_t cluster, const void* buffer) {
    if (cluster < 2) {
        return 0;
    }
    
    // Calculate sector number
    uint32_t sector = FAT16_DATA_START + (cluster - 2) * 2;
    
    // Write both sectors of the cluster
    const uint8_t* buf = (const uint8_t*)buffer;
    if (!ramdisk_write_sector(sector, buf)) {
        return 0;
    }
    if (!ramdisk_write_sector(sector + 1, buf + FAT16_SECTOR_SIZE)) {
        return 0;
    }
    
    return 1;
}

int fat16_create_file(const char* filename, const void* data, size_t size) {
    // Find free directory entry
    uint8_t dir_sector[FAT16_SECTOR_SIZE];
    fat16_dir_entry_t* entry = NULL;
    uint32_t entry_sector = 0;
    
    for (uint32_t sector = FAT16_ROOT_START; sector < FAT16_ROOT_START + FAT16_ROOT_SECTORS; sector++) {
        if (!ramdisk_read_sector(sector, dir_sector)) {
            return 0;
        }
        
        for (uint32_t i = 0; i < FAT16_SECTOR_SIZE / sizeof(fat16_dir_entry_t); i++) {
            fat16_dir_entry_t* dir_entry = (fat16_dir_entry_t*)&dir_sector[i * sizeof(fat16_dir_entry_t)];
            
            if (dir_entry->name[0] == 0x00 || dir_entry->name[0] == 0xE5) {
                // Free entry found
                entry = dir_entry;
                entry_sector = sector;
                break;
            }
        }
        
        if (entry) {
            break;
        }
    }
    
    if (!entry) {
        return 0; // No free directory entry
    }
    
    // Convert filename to FAT16 format
    char fatname[11];
    fat16_name_to_fatname(filename, fatname);
    
    // Create directory entry
    memset(entry, 0, sizeof(fat16_dir_entry_t));
    memcpy(entry->name, fatname, 11);
    entry->attributes = FAT16_ATTR_ARCHIVE;
    entry->file_size = size;
    
    // Allocate clusters for file data
    if (size > 0) {
        uint16_t first_cluster = fat16_find_free_cluster();
        if (first_cluster == 0) {
            return 0; // No free cluster
        }
        
        entry->cluster_low = first_cluster;
        
        // Write file data
        uint8_t cluster_data[FAT16_CLUSTER_SIZE];
        const uint8_t* file_data = (const uint8_t*)data;
        size_t remaining = size;
        uint16_t current_cluster = first_cluster;
        
        while (remaining > 0) {
            memset(cluster_data, 0, FAT16_CLUSTER_SIZE);
            size_t to_copy = remaining > FAT16_CLUSTER_SIZE ? FAT16_CLUSTER_SIZE : remaining;
            memcpy(cluster_data, file_data, to_copy);
            
            if (!fat16_write_cluster(current_cluster, cluster_data)) {
                return 0;
            }
            
            file_data += to_copy;
            remaining -= to_copy;
            
            if (remaining > 0) {
                // Need another cluster
                uint16_t next_cluster = fat16_find_free_cluster();
                if (next_cluster == 0) {
                    return 0; // No free cluster
                }
                
                fat16_set_fat_entry(current_cluster, next_cluster);
                current_cluster = next_cluster;
            } else {
                // Mark end of chain
                fat16_set_fat_entry(current_cluster, 0xFFFF);
            }
        }
    }
    
    // Write directory entry back to disk
    if (!ramdisk_write_sector(entry_sector, dir_sector)) {
        return 0;
    }
    
    return 1;
}

int fat16_list_files(void) {
    terminal_writestring("Directory listing:\n");
    
    uint8_t dir_sector[FAT16_SECTOR_SIZE];
    int file_count = 0;
    
    for (uint32_t sector = FAT16_ROOT_START; sector < FAT16_ROOT_START + FAT16_ROOT_SECTORS; sector++) {
        if (!ramdisk_read_sector(sector, dir_sector)) {
            return 0;
        }
        
        for (uint32_t i = 0; i < FAT16_SECTOR_SIZE / sizeof(fat16_dir_entry_t); i++) {
            fat16_dir_entry_t* entry = (fat16_dir_entry_t*)&dir_sector[i * sizeof(fat16_dir_entry_t)];
            
            if (entry->name[0] == 0x00) {
                break; // End of directory
            }
            
            if (entry->name[0] == 0xE5) {
                continue; // Deleted entry
            }
            
            if (entry->attributes & FAT16_ATTR_VOLUME_LABEL) {
                continue; // Volume label
            }
            
            // Print filename
            char filename[13];
            int j = 0;
            
            // Copy name
            for (int k = 0; k < 8 && entry->name[k] != ' '; k++) {
                filename[j++] = entry->name[k];
            }
            
            // Add extension if present
            if (entry->ext[0] != ' ') {
                filename[j++] = '.';
                for (int k = 0; k < 3 && entry->ext[k] != ' '; k++) {
                    filename[j++] = entry->ext[k];
                }
            }
            
            filename[j] = '\0';
            
            terminal_writestring("  ");
            terminal_writestring(filename);
            terminal_writestring(" (");
            
            char size_str[16];
            uint32_to_string(entry->file_size, size_str);
            terminal_writestring(size_str);
            terminal_writestring(" bytes)\n");
            
            file_count++;
        }
    }
    
    if (file_count == 0) {
        terminal_writestring("  No files found.\n");
    }
    
    return 1;
}

int fat16_read_file(const char* filename, void* buffer, size_t max_size) {
    // Convert filename to FAT16 format
    char fatname[11];
    fat16_name_to_fatname(filename, fatname);
    
    // Find file in root directory
    uint8_t dir_sector[FAT16_SECTOR_SIZE];
    fat16_dir_entry_t* entry = NULL;
    
    for (uint32_t sector = FAT16_ROOT_START; sector < FAT16_ROOT_START + FAT16_ROOT_SECTORS; sector++) {
        if (!ramdisk_read_sector(sector, dir_sector)) {
            return 0;
        }
        
        for (uint32_t i = 0; i < FAT16_SECTOR_SIZE / sizeof(fat16_dir_entry_t); i++) {
            fat16_dir_entry_t* dir_entry = (fat16_dir_entry_t*)&dir_sector[i * sizeof(fat16_dir_entry_t)];
            
            if (dir_entry->name[0] == 0x00) {
                break; // End of directory
            }
            
            if (fat16_name_compare(dir_entry->name, fatname) == 0) {
                entry = dir_entry;
                break;
            }
        }
        
        if (entry) {
            break;
        }
    }
    
    if (!entry) {
        return 0; // File not found
    }
    
    // Read file data
    size_t bytes_read = 0;
    uint8_t* buf = (uint8_t*)buffer;
    uint16_t cluster = entry->cluster_low;
    
    while (cluster >= 2 && cluster < FAT16_CLUSTER_END && bytes_read < max_size) {
        uint8_t cluster_data[FAT16_CLUSTER_SIZE];
        if (!fat16_read_cluster(cluster, cluster_data)) {
            break;
        }
        
        size_t to_copy = max_size - bytes_read;
        if (to_copy > FAT16_CLUSTER_SIZE) {
            to_copy = FAT16_CLUSTER_SIZE;
        }
        
        if (bytes_read + to_copy > entry->file_size) {
            to_copy = entry->file_size - bytes_read;
        }
        
        memcpy(buf + bytes_read, cluster_data, to_copy);
        bytes_read += to_copy;
        
        if (bytes_read >= entry->file_size) {
            break;
        }
        
        cluster = fat16_get_fat_entry(cluster);
    }
    
    return bytes_read;
}

int fat16_get_file_size(const char* filename) {
    // Convert filename to FAT16 format
    char fatname[11];
    fat16_name_to_fatname(filename, fatname);
    
    // Find file in root directory
    uint8_t dir_sector[FAT16_SECTOR_SIZE];
    
    for (uint32_t sector = FAT16_ROOT_START; sector < FAT16_ROOT_START + FAT16_ROOT_SECTORS; sector++) {
        if (!ramdisk_read_sector(sector, dir_sector)) {
            return -1;
        }
        
        for (uint32_t i = 0; i < FAT16_SECTOR_SIZE / sizeof(fat16_dir_entry_t); i++) {
            fat16_dir_entry_t* entry = (fat16_dir_entry_t*)&dir_sector[i * sizeof(fat16_dir_entry_t)];
            
            if (entry->name[0] == 0x00) {
                break; // End of directory
            }
            
            if (fat16_name_compare(entry->name, fatname) == 0) {
                return entry->file_size;
            }
        }
    }
    
    return -1; // File not found
}

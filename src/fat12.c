#include "fat12.h"
#include "ramdisk.h"
#include "memory.h"
#include "terminal.h"

// FAT12 layout in our ramdisk:
// Sector 0: Boot sector
// Sector 1-5: FAT1 (5 sectors for 512KB disk)
// Sector 6-10: FAT2 (backup)
// Sector 11-24: Root directory (14 sectors, 224 entries)
// Sector 25+: Data clusters

#define FAT12_BOOT_SECTOR     0
#define FAT12_FAT1_START      1
#define FAT12_FAT2_START      6
#define FAT12_ROOT_START      11
#define FAT12_DATA_START      25

#define FAT12_SECTORS_PER_FAT 5
#define FAT12_ROOT_SECTORS    14

// Helper function to convert string to uppercase FAT12 format
static void fat12_name_to_fatname(const char* name, char* fatname) {
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

// Helper function to compare FAT12 names
static int fat12_name_compare(const uint8_t* fatname1, const char* fatname2) {
    return memcmp(fatname1, fatname2, 11);
}

int fat12_format(void) {
    ramdisk_t* rd = get_ramdisk();
    if (!rd || !rd->initialized) {
        return 0;
    }
    
    // Create boot sector
    fat12_boot_sector_t boot_sector;
    memset(&boot_sector, 0, sizeof(boot_sector));
    
    // Jump instruction (3 bytes)
    boot_sector.jump[0] = 0xEB;
    boot_sector.jump[1] = 0x3C;
    boot_sector.jump[2] = 0x90;
    
    // OEM name
    memcpy(boot_sector.oem_name, "RutraOS ", 8);
    
    // BPB (BIOS Parameter Block)
    boot_sector.bytes_per_sector = FAT12_SECTOR_SIZE;
    boot_sector.sectors_per_cluster = 2;  // 1KB clusters
    boot_sector.reserved_sectors = 1;
    boot_sector.fat_count = 2;
    boot_sector.root_entries = FAT12_ROOT_ENTRIES;
    boot_sector.total_sectors = RAMDISK_SECTOR_COUNT;
    boot_sector.media_descriptor = FAT12_MEDIA_BYTE;
    boot_sector.sectors_per_fat = FAT12_SECTORS_PER_FAT;
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
    memcpy(boot_sector.filesystem_type, "FAT12   ", 8);
    
    // Boot signature
    boot_sector.boot_signature = FAT12_SIGNATURE;
    
    // Write boot sector
    if (!ramdisk_write_sector(FAT12_BOOT_SECTOR, &boot_sector)) {
        return 0;
    }
    
    // Initialize FAT tables
    uint8_t fat_sector[FAT12_SECTOR_SIZE];
    memset(fat_sector, 0, FAT12_SECTOR_SIZE);
    
    // First FAT sector has special entries
    fat_sector[0] = FAT12_MEDIA_BYTE;    // Media descriptor
    fat_sector[1] = 0xFF;                // End of chain marker
    fat_sector[2] = 0xFF;                // End of chain marker
    
    // Write first sector of both FATs
    ramdisk_write_sector(FAT12_FAT1_START, fat_sector);
    ramdisk_write_sector(FAT12_FAT2_START, fat_sector);
    
    // Clear remaining FAT sectors
    memset(fat_sector, 0, FAT12_SECTOR_SIZE);
    for (int i = 1; i < FAT12_SECTORS_PER_FAT; i++) {
        ramdisk_write_sector(FAT12_FAT1_START + i, fat_sector);
        ramdisk_write_sector(FAT12_FAT2_START + i, fat_sector);
    }
    
    // Clear root directory
    uint8_t root_sector[FAT12_SECTOR_SIZE];
    memset(root_sector, 0, FAT12_SECTOR_SIZE);
    for (int i = 0; i < FAT12_ROOT_SECTORS; i++) {
        ramdisk_write_sector(FAT12_ROOT_START + i, root_sector);
    }
    
    return 1;
}

uint16_t fat12_get_fat_entry(uint16_t cluster) {
    if (cluster < 2) {
        return 0;
    }
    
    // Calculate byte offset in FAT
    uint32_t byte_offset = (cluster * 3) / 2;
    uint32_t sector = FAT12_FAT1_START + (byte_offset / FAT12_SECTOR_SIZE);
    uint32_t sector_offset = byte_offset % FAT12_SECTOR_SIZE;
    
    uint8_t fat_sector[FAT12_SECTOR_SIZE];
    if (!ramdisk_read_sector(sector, fat_sector)) {
        return 0;
    }
    
    uint16_t value;
    if (cluster & 1) {
        // Odd cluster: take high 12 bits
        value = fat_sector[sector_offset] >> 4;
        if (sector_offset + 1 < FAT12_SECTOR_SIZE) {
            value |= (uint16_t)fat_sector[sector_offset + 1] << 4;
        } else {
            // Need to read next sector
            uint8_t next_sector[FAT12_SECTOR_SIZE];
            if (ramdisk_read_sector(sector + 1, next_sector)) {
                value |= (uint16_t)next_sector[0] << 4;
            }
        }
    } else {
        // Even cluster: take low 12 bits
        value = fat_sector[sector_offset];
        if (sector_offset + 1 < FAT12_SECTOR_SIZE) {
            value |= ((uint16_t)fat_sector[sector_offset + 1] & 0x0F) << 8;
        } else {
            // Need to read next sector
            uint8_t next_sector[FAT12_SECTOR_SIZE];
            if (ramdisk_read_sector(sector + 1, next_sector)) {
                value |= ((uint16_t)next_sector[0] & 0x0F) << 8;
            }
        }
    }
    
    return value;
}

void fat12_set_fat_entry(uint16_t cluster, uint16_t value) {
    if (cluster < 2) {
        return;
    }
    
    // Calculate byte offset in FAT
    uint32_t byte_offset = (cluster * 3) / 2;
    uint32_t sector = FAT12_FAT1_START + (byte_offset / FAT12_SECTOR_SIZE);
    uint32_t sector_offset = byte_offset % FAT12_SECTOR_SIZE;
    
    uint8_t fat_sector[FAT12_SECTOR_SIZE];
    if (!ramdisk_read_sector(sector, fat_sector)) {
        return;
    }
    
    if (cluster & 1) {
        // Odd cluster: set high 12 bits
        fat_sector[sector_offset] = (fat_sector[sector_offset] & 0x0F) | ((value & 0x0F) << 4);
        if (sector_offset + 1 < FAT12_SECTOR_SIZE) {
            fat_sector[sector_offset + 1] = (value >> 4) & 0xFF;
        } else {
            // Need to update next sector
            ramdisk_write_sector(sector, fat_sector);
            uint8_t next_sector[FAT12_SECTOR_SIZE];
            if (ramdisk_read_sector(sector + 1, next_sector)) {
                next_sector[0] = (value >> 4) & 0xFF;
                ramdisk_write_sector(sector + 1, next_sector);
            }
            // Update backup FAT
            ramdisk_write_sector(sector + FAT12_SECTORS_PER_FAT, fat_sector);
            ramdisk_write_sector(sector + 1 + FAT12_SECTORS_PER_FAT, next_sector);
            return;
        }
    } else {
        // Even cluster: set low 12 bits
        fat_sector[sector_offset] = value & 0xFF;
        if (sector_offset + 1 < FAT12_SECTOR_SIZE) {
            fat_sector[sector_offset + 1] = (fat_sector[sector_offset + 1] & 0xF0) | ((value >> 8) & 0x0F);
        } else {
            // Need to update next sector
            ramdisk_write_sector(sector, fat_sector);
            uint8_t next_sector[FAT12_SECTOR_SIZE];
            if (ramdisk_read_sector(sector + 1, next_sector)) {
                next_sector[0] = (next_sector[0] & 0xF0) | ((value >> 8) & 0x0F);
                ramdisk_write_sector(sector + 1, next_sector);
            }
            // Update backup FAT
            ramdisk_write_sector(sector + FAT12_SECTORS_PER_FAT, fat_sector);
            ramdisk_write_sector(sector + 1 + FAT12_SECTORS_PER_FAT, next_sector);
            return;
        }
    }
    
    // Write updated sector to both FATs
    ramdisk_write_sector(sector, fat_sector);
    ramdisk_write_sector(sector + FAT12_SECTORS_PER_FAT, fat_sector);
}

uint16_t fat12_find_free_cluster(void) {
    // Start from cluster 2 (first data cluster)
    for (uint16_t cluster = 2; cluster < 4085; cluster++) {
        if (fat12_get_fat_entry(cluster) == FAT12_CLUSTER_FREE) {
            return cluster;
        }
    }
    return 0; // No free cluster found
}

int fat12_read_cluster(uint16_t cluster, void* buffer) {
    if (cluster < 2) {
        return 0;
    }
    
    // Calculate sector number
    uint32_t sector = FAT12_DATA_START + (cluster - 2) * 2;
    
    // Read both sectors of the cluster
    uint8_t* buf = (uint8_t*)buffer;
    if (!ramdisk_read_sector(sector, buf)) {
        return 0;
    }
    if (!ramdisk_read_sector(sector + 1, buf + FAT12_SECTOR_SIZE)) {
        return 0;
    }
    
    return 1;
}

int fat12_write_cluster(uint16_t cluster, const void* buffer) {
    if (cluster < 2) {
        return 0;
    }
    
    // Calculate sector number
    uint32_t sector = FAT12_DATA_START + (cluster - 2) * 2;
    
    // Write both sectors of the cluster
    const uint8_t* buf = (const uint8_t*)buffer;
    if (!ramdisk_write_sector(sector, buf)) {
        return 0;
    }
    if (!ramdisk_write_sector(sector + 1, buf + FAT12_SECTOR_SIZE)) {
        return 0;
    }
    
    return 1;
}

int fat12_create_file(const char* filename, const void* data, size_t size) {
    // Find free directory entry
    uint8_t dir_sector[FAT12_SECTOR_SIZE];
    fat12_dir_entry_t* entry = NULL;
    uint32_t entry_sector = 0;
    
    for (uint32_t sector = FAT12_ROOT_START; sector < FAT12_ROOT_START + FAT12_ROOT_SECTORS; sector++) {
        if (!ramdisk_read_sector(sector, dir_sector)) {
            return 0;
        }
        
        for (uint32_t i = 0; i < FAT12_SECTOR_SIZE / sizeof(fat12_dir_entry_t); i++) {
            fat12_dir_entry_t* dir_entry = (fat12_dir_entry_t*)&dir_sector[i * sizeof(fat12_dir_entry_t)];
            
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
    
    // Convert filename to FAT12 format
    char fatname[11];
    fat12_name_to_fatname(filename, fatname);
    
    // Create directory entry
    memset(entry, 0, sizeof(fat12_dir_entry_t));
    memcpy(entry->name, fatname, 11);
    entry->attributes = FAT12_ATTR_ARCHIVE;
    entry->file_size = size;
    
    // Allocate clusters for file data
    if (size > 0) {
        uint16_t first_cluster = fat12_find_free_cluster();
        if (first_cluster == 0) {
            return 0; // No free cluster
        }
        
        entry->cluster_low = first_cluster;
        
        // Write file data
        uint8_t cluster_data[FAT12_CLUSTER_SIZE];
        const uint8_t* file_data = (const uint8_t*)data;
        size_t remaining = size;
        uint16_t current_cluster = first_cluster;
        
        while (remaining > 0) {
            memset(cluster_data, 0, FAT12_CLUSTER_SIZE);
            size_t to_copy = remaining > FAT12_CLUSTER_SIZE ? FAT12_CLUSTER_SIZE : remaining;
            memcpy(cluster_data, file_data, to_copy);
            
            if (!fat12_write_cluster(current_cluster, cluster_data)) {
                return 0;
            }
            
            file_data += to_copy;
            remaining -= to_copy;
            
            if (remaining > 0) {
                // Need another cluster
                uint16_t next_cluster = fat12_find_free_cluster();
                if (next_cluster == 0) {
                    return 0; // No free cluster
                }
                
                fat12_set_fat_entry(current_cluster, next_cluster);
                current_cluster = next_cluster;
            } else {
                // Mark end of chain
                fat12_set_fat_entry(current_cluster, 0xFFF);
            }
        }
    }
    
    // Write directory entry back to disk
    if (!ramdisk_write_sector(entry_sector, dir_sector)) {
        return 0;
    }
    
    return 1;
}

int fat12_list_files(void) {
    terminal_writestring("Directory listing:\n");
    
    uint8_t dir_sector[FAT12_SECTOR_SIZE];
    int file_count = 0;
    
    for (uint32_t sector = FAT12_ROOT_START; sector < FAT12_ROOT_START + FAT12_ROOT_SECTORS; sector++) {
        if (!ramdisk_read_sector(sector, dir_sector)) {
            return 0;
        }
        
        for (uint32_t i = 0; i < FAT12_SECTOR_SIZE / sizeof(fat12_dir_entry_t); i++) {
            fat12_dir_entry_t* entry = (fat12_dir_entry_t*)&dir_sector[i * sizeof(fat12_dir_entry_t)];
            
            if (entry->name[0] == 0x00) {
                break; // End of directory
            }
            
            if (entry->name[0] == 0xE5) {
                continue; // Deleted entry
            }
            
            if (entry->attributes & FAT12_ATTR_VOLUME_LABEL) {
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

int fat12_read_file(const char* filename, void* buffer, size_t max_size) {
    // Convert filename to FAT12 format
    char fatname[11];
    fat12_name_to_fatname(filename, fatname);
    
    // Find file in root directory
    uint8_t dir_sector[FAT12_SECTOR_SIZE];
    fat12_dir_entry_t* entry = NULL;
    
    for (uint32_t sector = FAT12_ROOT_START; sector < FAT12_ROOT_START + FAT12_ROOT_SECTORS; sector++) {
        if (!ramdisk_read_sector(sector, dir_sector)) {
            return 0;
        }
        
        for (uint32_t i = 0; i < FAT12_SECTOR_SIZE / sizeof(fat12_dir_entry_t); i++) {
            fat12_dir_entry_t* dir_entry = (fat12_dir_entry_t*)&dir_sector[i * sizeof(fat12_dir_entry_t)];
            
            if (dir_entry->name[0] == 0x00) {
                break; // End of directory
            }
            
            if (fat12_name_compare(dir_entry->name, fatname) == 0) {
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
    
    while (cluster >= 2 && cluster < 0xFF8 && bytes_read < max_size) {
        uint8_t cluster_data[FAT12_CLUSTER_SIZE];
        if (!fat12_read_cluster(cluster, cluster_data)) {
            break;
        }
        
        size_t to_copy = max_size - bytes_read;
        if (to_copy > FAT12_CLUSTER_SIZE) {
            to_copy = FAT12_CLUSTER_SIZE;
        }
        
        if (bytes_read + to_copy > entry->file_size) {
            to_copy = entry->file_size - bytes_read;
        }
        
        memcpy(buf + bytes_read, cluster_data, to_copy);
        bytes_read += to_copy;
        
        if (bytes_read >= entry->file_size) {
            break;
        }
        
        cluster = fat12_get_fat_entry(cluster);
    }
    
    return bytes_read;
}

int fat12_get_file_size(const char* filename) {
    // Convert filename to FAT12 format
    char fatname[11];
    fat12_name_to_fatname(filename, fatname);
    
    // Find file in root directory
    uint8_t dir_sector[FAT12_SECTOR_SIZE];
    
    for (uint32_t sector = FAT12_ROOT_START; sector < FAT12_ROOT_START + FAT12_ROOT_SECTORS; sector++) {
        if (!ramdisk_read_sector(sector, dir_sector)) {
            return -1;
        }
        
        for (uint32_t i = 0; i < FAT12_SECTOR_SIZE / sizeof(fat12_dir_entry_t); i++) {
            fat12_dir_entry_t* entry = (fat12_dir_entry_t*)&dir_sector[i * sizeof(fat12_dir_entry_t)];
            
            if (entry->name[0] == 0x00) {
                break; // End of directory
            }
            
            if (fat12_name_compare(entry->name, fatname) == 0) {
                return entry->file_size;
            }
        }
    }
    
    return -1; // File not found
}

#include "fat16.h"
#include "ramdisk.h"
#include "memory.h"
#include "terminal.h"
#include "string.h"
#include "memory_utils.h"
#include "string.h"

// Current directory tracking
static char current_directory[256] = "/";
static uint16_t current_directory_cluster = 0;  // 0 means root directory

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

uint16_t fat16_find_free_cluster(void) {
    // Start from cluster 2 (first data cluster)
    // Simplified: just find first unused cluster
    static uint16_t last_allocated = 2;
    
    for (uint16_t cluster = last_allocated; cluster < 256; cluster++) {
        // For simplicity, assume clusters are allocated sequentially
        // and don't check FAT (works for our simple use case)
        last_allocated = cluster + 1;
        return cluster;
    }
    return 0;
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
    // Find free directory entry in current directory
    uint8_t dir_sector[FAT16_SECTOR_SIZE];
    fat16_dir_entry_t* entry = NULL;
    uint32_t entry_sector = 0;
    uint32_t start_sector = (current_directory_cluster == 0) ? FAT16_ROOT_START : 
                           FAT16_DATA_START + (current_directory_cluster - 2) * 2;
    uint32_t sectors_to_read = (current_directory_cluster == 0) ? FAT16_ROOT_SECTORS : 2;
    
    for (uint32_t sector = 0; sector < sectors_to_read; sector++) {
        uint32_t actual_sector = start_sector + sector;
        if (!ramdisk_read_sector(actual_sector, dir_sector)) {
            return 0;
        }
        
        for (uint32_t i = 0; i < FAT16_SECTOR_SIZE / sizeof(fat16_dir_entry_t); i++) {
            fat16_dir_entry_t* dir_entry = (fat16_dir_entry_t*)&dir_sector[i * sizeof(fat16_dir_entry_t)];
            
            if (dir_entry->name[0] == 0x00 || dir_entry->name[0] == 0xE5) {
                entry = dir_entry;
                entry_sector = actual_sector;
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
    
    // Simplified: allocate contiguous clusters
    if (size > 0) {
        uint16_t clusters_needed = (size + FAT16_CLUSTER_SIZE - 1) / FAT16_CLUSTER_SIZE;
        uint16_t first_cluster = fat16_find_free_cluster();
        if (first_cluster == 0) {
            return 0;
        }
        
        entry->cluster_low = first_cluster;
        
        // Write file data to contiguous clusters
        const uint8_t* file_data = (const uint8_t*)data;
        for (uint16_t i = 0; i < clusters_needed; i++) {
            uint8_t cluster_data[FAT16_CLUSTER_SIZE];
            memset(cluster_data, 0, FAT16_CLUSTER_SIZE);
            
            size_t offset = i * FAT16_CLUSTER_SIZE;
            size_t to_copy = (size - offset > FAT16_CLUSTER_SIZE) ? FAT16_CLUSTER_SIZE : (size - offset);
            memcpy(cluster_data, file_data + offset, to_copy);
            
            if (!fat16_write_cluster(first_cluster + i, cluster_data)) {
                return 0;
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
    return fat16_list_directory(".");
}

int fat16_read_file(const char* filename, void* buffer, size_t max_size) {
    // Convert filename to FAT16 format
    char fatname[11];
    fat16_name_to_fatname(filename, fatname);
    
    // Find file in current directory
    uint8_t dir_sector[FAT16_SECTOR_SIZE];
    fat16_dir_entry_t* entry = NULL;
    uint32_t start_sector = (current_directory_cluster == 0) ? FAT16_ROOT_START : 
                           FAT16_DATA_START + (current_directory_cluster - 2) * 2;
    uint32_t sectors_to_read = (current_directory_cluster == 0) ? FAT16_ROOT_SECTORS : 2;
    
    for (uint32_t sector = 0; sector < sectors_to_read; sector++) {
        if (!ramdisk_read_sector(start_sector + sector, dir_sector)) {
            return 0;
        }
        
        for (uint32_t i = 0; i < FAT16_SECTOR_SIZE / sizeof(fat16_dir_entry_t); i++) {
            fat16_dir_entry_t* dir_entry = (fat16_dir_entry_t*)&dir_sector[i * sizeof(fat16_dir_entry_t)];
            
            if (dir_entry->name[0] == 0x00) {
                break;
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
    
    // Simplified: read contiguous clusters
    size_t bytes_to_read = (entry->file_size < max_size) ? entry->file_size : max_size;
    uint16_t clusters_needed = (bytes_to_read + FAT16_CLUSTER_SIZE - 1) / FAT16_CLUSTER_SIZE;
    uint8_t* buf = (uint8_t*)buffer;
    
    for (uint16_t i = 0; i < clusters_needed; i++) {
        uint8_t cluster_data[FAT16_CLUSTER_SIZE];
        if (!fat16_read_cluster(entry->cluster_low + i, cluster_data)) {
            return i * FAT16_CLUSTER_SIZE;
        }
        
        size_t offset = i * FAT16_CLUSTER_SIZE;
        size_t to_copy = (bytes_to_read - offset > FAT16_CLUSTER_SIZE) ? FAT16_CLUSTER_SIZE : (bytes_to_read - offset);
        memcpy(buf + offset, cluster_data, to_copy);
    }
    
    return bytes_to_read;
}

int fat16_get_file_size(const char* filename) {
    // Convert filename to FAT16 format
    char fatname[11];
    fat16_name_to_fatname(filename, fatname);
    
    // Find file in current directory
    uint8_t dir_sector[FAT16_SECTOR_SIZE];
    uint32_t start_sector = (current_directory_cluster == 0) ? FAT16_ROOT_START : 
                           FAT16_DATA_START + (current_directory_cluster - 2) * 2;
    uint32_t sectors_to_read = (current_directory_cluster == 0) ? FAT16_ROOT_SECTORS : 2;
    
    for (uint32_t sector = 0; sector < sectors_to_read; sector++) {
        if (!ramdisk_read_sector(start_sector + sector, dir_sector)) {
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

// Directory support functions

// Get current directory path
char* fat16_get_current_directory(void) {
    return current_directory;
}

// Parse a path into components
int fat16_parse_path(const char* path, char* components[], int max_components) {
    if (!path || !components) {
        return 0;
    }
    
    int count = 0;
    char* path_copy = (char*)kmalloc(strlen(path) + 1);
    strcpy(path_copy, path);
    
    char* token = strtok(path_copy, "/");
    while (token && count < max_components) {
        if (strcmp(token, ".") != 0) {  // Skip current directory references
            if (strcmp(token, "..") == 0) {
                // Parent directory - remove last component if possible
                if (count > 0) {
                    kfree(components[count - 1]);
                    count--;
                }
            } else {
                components[count] = (char*)kmalloc(strlen(token) + 1);
                strcpy(components[count], token);
                count++;
            }
        }
        token = strtok(NULL, "/");
    }
    
    kfree(path_copy);
    return count;
}

// Create a new directory
int fat16_create_directory(const char* dirname) {
    if (!dirname || strlen(dirname) == 0) {
        return 0;
    }
    
    // Find free directory entry in current directory
    uint8_t dir_sector[FAT16_SECTOR_SIZE];
    fat16_dir_entry_t* entry = NULL;
    uint32_t entry_sector = 0;
    uint32_t start_sector = (current_directory_cluster == 0) ? FAT16_ROOT_START : 
                           FAT16_DATA_START + (current_directory_cluster - 2) * 2;
    uint32_t sectors_to_read = (current_directory_cluster == 0) ? FAT16_ROOT_SECTORS : 2;
    
    for (uint32_t sector = 0; sector < sectors_to_read; sector++) {
        uint32_t actual_sector = start_sector + sector;
        if (!ramdisk_read_sector(actual_sector, dir_sector)) {
            return 0;
        }
        
        for (uint32_t i = 0; i < FAT16_SECTOR_SIZE / sizeof(fat16_dir_entry_t); i++) {
            fat16_dir_entry_t* dir_entry = (fat16_dir_entry_t*)&dir_sector[i * sizeof(fat16_dir_entry_t)];
            
            if (dir_entry->name[0] == 0x00 || dir_entry->name[0] == 0xE5) {
                entry = dir_entry;
                entry_sector = actual_sector;
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
    
    // Allocate a cluster for the new directory
    uint16_t dir_cluster = fat16_find_free_cluster();
    if (dir_cluster == 0) {
        return 0;
    }
    
    // Create directory entry
    char fatname[11];
    fat16_name_to_fatname(dirname, fatname);
    
    memset(entry, 0, sizeof(fat16_dir_entry_t));
    memcpy(entry->name, fatname, 11);
    entry->attributes = FAT16_ATTR_DIRECTORY;
    entry->cluster_low = dir_cluster;
    entry->file_size = 0; // Directories have size 0
    
    // Write directory entry back to disk
    if (!ramdisk_write_sector(entry_sector, dir_sector)) {
        return 0;
    }
    
    // Initialize the new directory with . and .. entries
    uint8_t new_dir_data[FAT16_CLUSTER_SIZE];
    memset(new_dir_data, 0, FAT16_CLUSTER_SIZE);
    
    // Create "." entry (current directory)
    fat16_dir_entry_t* dot_entry = (fat16_dir_entry_t*)new_dir_data;
    memset(dot_entry->name, ' ', 11);
    dot_entry->name[0] = '.';
    dot_entry->attributes = FAT16_ATTR_DIRECTORY;
    dot_entry->cluster_low = dir_cluster;
    
    // Create ".." entry (parent directory)
    fat16_dir_entry_t* dotdot_entry = (fat16_dir_entry_t*)(new_dir_data + sizeof(fat16_dir_entry_t));
    memset(dotdot_entry->name, ' ', 11);
    dotdot_entry->name[0] = '.';
    dotdot_entry->name[1] = '.';
    dotdot_entry->attributes = FAT16_ATTR_DIRECTORY;
    dotdot_entry->cluster_low = current_directory_cluster; // Parent cluster (0 for root)
    
    // Write the new directory data
    if (!fat16_write_cluster(dir_cluster, new_dir_data)) {
        return 0;
    }
    
    return 1;
}

// Change current directory
int fat16_change_directory(const char* path) {
    if (!path) {
        return 0;
    }
    
    // Handle special cases
    if (strcmp(path, ".") == 0) {
        return 1; // Stay in current directory
    }
    
    if (strcmp(path, "..") == 0) {
        // Go to parent directory
        if (current_directory_cluster == 0) {
            return 1; // Already at root
        }
        
        // Look for ".." entry in current directory
        uint8_t dir_sector[FAT16_SECTOR_SIZE];
        uint32_t start_sector = FAT16_DATA_START + (current_directory_cluster - 2) * 2;
        
        if (ramdisk_read_sector(start_sector, dir_sector)) {
            fat16_dir_entry_t* dotdot_entry = (fat16_dir_entry_t*)(dir_sector + sizeof(fat16_dir_entry_t));
            current_directory_cluster = dotdot_entry->cluster_low;
            
            // Update path string
            char* last_slash = strrchr(current_directory, '/');
            if (last_slash && last_slash != current_directory) {
                *last_slash = '\0';
            } else {
                strcpy(current_directory, "/");
            }
        }
        return 1;
    }
    
    // Handle absolute or complex paths (simplified - only basic paths supported)
    if (path[0] == '/') {
        current_directory_cluster = 0;
        strcpy(current_directory, "/");
        return 1;
    }
    
    // Simple subdirectory navigation
    char fatname[11];
    fat16_name_to_fatname(path, fatname);
    
    uint8_t dir_sector[FAT16_SECTOR_SIZE];
    uint32_t start_sector = (current_directory_cluster == 0) ? FAT16_ROOT_START : 
                           FAT16_DATA_START + (current_directory_cluster - 2) * 2;
    uint32_t sectors_to_read = (current_directory_cluster == 0) ? FAT16_ROOT_SECTORS : 2;
    
    for (uint32_t sector = 0; sector < sectors_to_read; sector++) {
        if (!ramdisk_read_sector(start_sector + sector, dir_sector)) {
            return 0;
        }
        
        for (uint32_t i = 0; i < FAT16_SECTOR_SIZE / sizeof(fat16_dir_entry_t); i++) {
            fat16_dir_entry_t* entry = (fat16_dir_entry_t*)&dir_sector[i * sizeof(fat16_dir_entry_t)];
            
            if (entry->name[0] == 0x00) {
                break;
            }
            
            if (fat16_name_compare(entry->name, fatname) == 0 && 
                (entry->attributes & FAT16_ATTR_DIRECTORY)) {
                
                current_directory_cluster = entry->cluster_low;
                
                if (strcmp(current_directory, "/") != 0) {
                    strcat(current_directory, "/");
                }
                strcat(current_directory, path);
                
                return 1;
            }
        }
    }
    
    return 0;
}

// List directory contents (enhanced version)
int fat16_list_directory(const char* path) {
    uint16_t old_cluster = current_directory_cluster;
    char old_path[256];
    strcpy(old_path, current_directory);
    
    // Change to target directory if path is provided
    if (path && strlen(path) > 0 && strcmp(path, ".") != 0) {
        if (!fat16_change_directory(path)) {
            terminal_writestring("Directory not found: ");
            terminal_writestring(path);
            terminal_writestring("\n");
            return 0;
        }
    }
    
    terminal_writestring("Directory listing for ");
    terminal_writestring(current_directory);
    terminal_writestring(":\n");
    
    uint8_t dir_sector[FAT16_SECTOR_SIZE];
    int entry_count = 0;
    uint32_t start_sector = (current_directory_cluster == 0) ? FAT16_ROOT_START : 
                           FAT16_DATA_START + (current_directory_cluster - 2) * 2;
    uint32_t sectors_to_read = (current_directory_cluster == 0) ? FAT16_ROOT_SECTORS : 2;
    
    for (uint32_t sector = 0; sector < sectors_to_read; sector++) {
        if (!ramdisk_read_sector(start_sector + sector, dir_sector)) {
            return 0;
        }
        
        for (uint32_t i = 0; i < FAT16_SECTOR_SIZE / sizeof(fat16_dir_entry_t); i++) {
            fat16_dir_entry_t* entry = (fat16_dir_entry_t*)&dir_sector[i * sizeof(fat16_dir_entry_t)];
            
            if (entry->name[0] == 0x00) {
                goto end_listing; // End of directory
            }
            
            if (entry->name[0] == 0xE5) {
                continue; // Deleted entry
            }
            
            if (entry->attributes & FAT16_ATTR_VOLUME_LABEL) {
                continue; // Volume label
            }
            
            // Print entry
            terminal_writestring("  ");
            
            if (entry->attributes & FAT16_ATTR_DIRECTORY) {
                terminal_writestring("[DIR] ");
            } else {
                terminal_writestring("      ");
            }
            
            // Print filename
            char filename[13];
            int j = 0;
            
            // Copy name
            for (int k = 0; k < 8 && entry->name[k] != ' '; k++) {
                filename[j++] = entry->name[k];
            }
            
            // Add extension if present and not a directory
            if (!(entry->attributes & FAT16_ATTR_DIRECTORY) && entry->ext[0] != ' ') {
                filename[j++] = '.';
                for (int k = 0; k < 3 && entry->ext[k] != ' '; k++) {
                    filename[j++] = entry->ext[k];
                }
            }
            
            filename[j] = '\0';
            terminal_writestring(filename);
            
            if (!(entry->attributes & FAT16_ATTR_DIRECTORY)) {
                terminal_writestring(" (");
                char size_str[16];
                uint32_to_string(entry->file_size, size_str);
                terminal_writestring(size_str);
                terminal_writestring(" bytes)");
            }
            
            terminal_writestring("\n");
            entry_count++;
        }
    }
    
end_listing:
    if (entry_count == 0) {
        terminal_writestring("  No entries found.\n");
    }
    
    // Restore original directory if we changed it
    if (path && strlen(path) > 0 && strcmp(path, ".") != 0) {
        current_directory_cluster = old_cluster;
        strcpy(current_directory, old_path);
    }
    
    return 1;
}

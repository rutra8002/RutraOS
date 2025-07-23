#ifndef ATA_H
#define ATA_H
#include "types.h"

// Disk info struct
typedef struct {
    uint16_t io_base;
    uint16_t ctrl_base;
    int is_slave;
    char model[21];
    int present;
} ata_disk_t;

extern ata_disk_t ata_disks[4];
extern int ata_disk_count;

void ata_detect_disks();

#endif // ATA_H

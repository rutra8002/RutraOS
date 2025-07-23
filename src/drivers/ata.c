#include "ata.h"
#include "io.h"
#include "terminal.h"

// Basic ATA I/O ports
#define ATA_PRIMARY_IO      0x1F0
#define ATA_PRIMARY_CTRL    0x3F6
#define ATA_SECONDARY_IO    0x170
#define ATA_SECONDARY_CTRL  0x376

// Identify command
#define ATA_CMD_IDENTIFY    0xEC

// Disk info struct
ata_disk_t ata_disks[4];
int ata_disk_count = 0;

static int ata_identify_drive(uint16_t io, uint16_t ctrl, int is_slave) {
    // Select drive
    outb(io + 6, is_slave ? 0xB0 : 0xA0);
    // Small delay
    for (volatile int i = 0; i < 1000; i++);
    // Send IDENTIFY
    outb(io + 7, ATA_CMD_IDENTIFY);
    // Wait for response
    uint8_t status = inb(io + 7);
    if (status == 0) return 0; // No drive
    while ((status & 0x80) && !(status & 0x08)) status = inb(io + 7);
    // Read identification space
    uint16_t id[256];
    for (int i = 0; i < 256; i++) id[i] = inw(io);
    // Save info
    ata_disk_t* disk = &ata_disks[ata_disk_count++];
    disk->io_base = io;
    disk->ctrl_base = ctrl;
    disk->is_slave = is_slave;
    // Properly decode model string (40 bytes, swap bytes in each word)
    for (int i = 0; i < 20; i++) {
        disk->model[2*i] = (char)(id[27 + i] >> 8);
        disk->model[2*i+1] = (char)(id[27 + i] & 0xFF);
    }
    disk->model[40] = 0;
    disk->present = 1;
    return 1;
}

void ata_detect_disks() {
    ata_disk_count = 0;
    for (int ch = 0; ch < 2; ch++) {
        uint16_t io = (ch == 0) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
        uint16_t ctrl = (ch == 0) ? ATA_PRIMARY_CTRL : ATA_SECONDARY_CTRL;
        for (int sl = 0; sl < 2; sl++) {
            ata_identify_drive(io, ctrl, sl);
        }
    }
}

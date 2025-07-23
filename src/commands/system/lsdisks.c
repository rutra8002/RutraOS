#include "command.h"
#include "terminal.h"
#include "ata.h"

static int cmd_lsdisks_main(int argc, char** argv) {
    ata_detect_disks();
    terminal_writestring("Detected disks:\n");
    for (int i = 0; i < ata_disk_count; i++) {
        if (ata_disks[i].present) {
            terminal_writestring("  ");
            terminal_writestring(ata_disks[i].is_slave ? "Slave" : "Master");
            terminal_writestring(" on ");
            terminal_writestring((ata_disks[i].io_base == 0x1F0) ? "Primary" : "Secondary");
            terminal_writestring(": ");
            terminal_writestring(ata_disks[i].model);
            terminal_writestring("\n");
        }
    }
    if (ata_disk_count == 0) terminal_writestring("  (none)\n");
    return 0;
}

REGISTER_COMMAND("lsdisks", "List detected ATA disks", cmd_lsdisks_main)

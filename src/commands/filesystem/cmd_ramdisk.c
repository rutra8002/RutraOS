#include "command.h"
#include "terminal.h"
#include "ramdisk.h"

static int cmd_ramdisk_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("ramdisk", "");
        terminal_writestring("Display ramdisk information and statistics.\n");
        terminal_writestring("Shows the virtual disk status, filesystem type,\n");
        terminal_writestring("total size, and available space.\n");
        return 0;
    }
    
    ramdisk_print_info();
    return 0;
}

REGISTER_COMMAND("ramdisk", "Show ramdisk information", cmd_ramdisk_main)

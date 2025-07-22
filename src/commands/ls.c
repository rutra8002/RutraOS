#include "command.h"
#include "terminal.h"
#include "fat16.h"

static int cmd_ls_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("ls", "");
        terminal_writestring("List all files in the ramdisk.\n");
        return 0;
    }
    
    fat16_list_files();
    return 0;
}

REGISTER_COMMAND("ls", "List files in ramdisk", cmd_ls_main)

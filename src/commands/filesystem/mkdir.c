#include "command.h"
#include "terminal.h"
#include "fat16.h"

static int cmd_mkdir_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("mkdir", "<directory_name>");
        terminal_writestring("Create a new directory in the current directory.\n");
        return 0;
    }
    
    if (argc < 2) {
        terminal_writestring("mkdir: missing directory name\n");
        command_show_usage("mkdir", "<directory_name>");
        return 1;
    }
    
    if (fat16_create_directory(argv[1])) {
        terminal_writestring("Directory '");
        terminal_writestring(argv[1]);
        terminal_writestring("' created successfully.\n");
        return 0;
    } else {
        terminal_writestring("mkdir: failed to create directory '");
        terminal_writestring(argv[1]);
        terminal_writestring("'\n");
        return 1;
    }
}

REGISTER_COMMAND("mkdir", "Create a directory", cmd_mkdir_main)

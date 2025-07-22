#include "command.h"
#include "terminal.h"
#include "fat16.h"

static int cmd_ls_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("ls", "[directory]");
        terminal_writestring("List files and directories.\n");
        terminal_writestring("If no directory is specified, lists current directory.\n");
        return 0;
    }
    
    const char* path = ".";  // Default to current directory
    if (argc >= 2) {
        path = argv[1];
    }
    
    fat16_list_directory(path);
    return 0;
}

REGISTER_COMMAND("ls", "List files and directories", cmd_ls_main)

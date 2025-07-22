#include "command.h"
#include "terminal.h"

static int cmd_help_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("help", "");
        terminal_writestring("Show all available commands with descriptions.\n");
        return 0;
    }
    
    command_list_available();
    terminal_writestring("\nUse '<command> --help' for specific command help.\n");
    
    return 0;
}

REGISTER_COMMAND("help", "Show available commands", cmd_help_main)

#include "command.h"
#include "terminal.h"
#include "process.h"

static int cmd_ps_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("ps", "");
        terminal_writestring("List all running processes with their status.\n");
        return 0;
    }
    
    process_list();
    return 0;
}

REGISTER_COMMAND("ps", "List all processes", cmd_ps_main)

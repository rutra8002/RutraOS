#include "command.h"
#include "terminal.h"

static int cmd_clear_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("clear", "");
        terminal_writestring("Clear the terminal screen.\n");
        return 0;
    }
    
    terminal_clear();
    return 0;
}

REGISTER_COMMAND("clear", "Clear the terminal screen", cmd_clear_main)

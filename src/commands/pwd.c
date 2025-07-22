#include "command.h"
#include "terminal.h"
#include "fat16.h"

static int cmd_pwd_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("pwd", "");
        terminal_writestring("Print current working directory.\n");
        return 0;
    }
    
    terminal_writestring(fat16_get_current_directory());
    terminal_writestring("\n");
    return 0;
}

REGISTER_COMMAND("pwd", "Print working directory", cmd_pwd_main)

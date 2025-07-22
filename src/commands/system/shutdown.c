#include "command.h"
#include "terminal.h"
#include "shell.h"

static int cmd_shutdown_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("shutdown", "");
        terminal_writestring("Shut down the operating system safely.\n");
        terminal_writestring("This will halt the system and power off if supported.\n");
        return 0;
    }
    
    shell_shutdown();
    return 0;
}

REGISTER_COMMAND("shutdown", "Shut down the system", cmd_shutdown_main)

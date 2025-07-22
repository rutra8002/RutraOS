#include "command.h"
#include "terminal.h"
#include "shell.h"

static int cmd_reboot_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("reboot", "");
        terminal_writestring("Restart the operating system.\n");
        terminal_writestring("This will reboot the system immediately.\n");
        return 0;
    }
    
    shell_reboot();
    return 0;
}

REGISTER_COMMAND("reboot", "Restart the system", cmd_reboot_main)

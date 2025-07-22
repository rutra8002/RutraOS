#include "command.h"
#include "terminal.h"
#include "fat16.h"

static int cmd_cd_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("cd", "[directory_path]");
        terminal_writestring("Change current directory. Use '/' for root, '..' for parent directory.\n");
        return 0;
    }
    
    const char* path = "/";  // Default to root if no argument
    if (argc >= 2) {
        path = argv[1];
    }
    
    if (fat16_change_directory(path)) {
        terminal_writestring("Changed directory to: ");
        terminal_writestring(fat16_get_current_directory());
        terminal_writestring("\n");
        return 0;
    } else {
        terminal_writestring("cd: cannot access directory '");
        terminal_writestring(path);
        terminal_writestring("': No such directory\n");
        return 1;
    }
}

REGISTER_COMMAND("cd", "Change directory", cmd_cd_main)

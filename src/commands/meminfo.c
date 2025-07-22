#include "command.h"
#include "terminal.h"
#include "memory.h"

static int cmd_meminfo_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("meminfo", "");
        terminal_writestring("Display current memory usage statistics.\n");
        return 0;
    }
    
    memory_print_stats();
    return 0;
}

REGISTER_COMMAND("meminfo", "Show memory information", cmd_meminfo_main)

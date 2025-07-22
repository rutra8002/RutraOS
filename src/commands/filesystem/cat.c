#include "command.h"
#include "terminal.h"
#include "fat16.h"
#include "string.h"

static int cmd_cat_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("cat", "<filename>");
        terminal_writestring("Display the contents of a file.\n");
        return 0;
    }
    
    if (argc < 2) {
        terminal_writestring("Usage: cat <filename>\n");
        return 1;
    }
    
    char file_buffer[4096];
    int size = fat16_read_file(argv[1], file_buffer, sizeof(file_buffer) - 1);
    if (size > 0) {
        file_buffer[size] = '\0';
        terminal_writestring("File contents:\n");
        terminal_writestring(file_buffer);
        terminal_writestring("\n");
        return 0;
    } else {
        terminal_writestring("File not found or read error\n");
        return 1;
    }
}

REGISTER_COMMAND("cat", "Display file contents", cmd_cat_main)

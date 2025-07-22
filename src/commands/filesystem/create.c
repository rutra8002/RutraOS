#include "command.h"
#include "terminal.h"
#include "fat16.h"
#include "string.h"

static int cmd_create_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("create", "<filename> <content>");
        terminal_writestring("Create a new file with the specified content.\n");
        terminal_writestring("All arguments after filename are treated as content.\n");
        return 0;
    }
    
    if (argc < 3) {
        terminal_writestring("Usage: create <filename> <content>\n");
        return 1;
    }
    
    // Combine all arguments after filename as content
    char content_buffer[1024];
    content_buffer[0] = '\0';
    
    for (int i = 2; i < argc; i++) {
        if (i > 2) {
            strcat(content_buffer, " ");
        }
        strcat(content_buffer, argv[i]);
    }
    
    if (fat16_create_file(argv[1], content_buffer, strlen(content_buffer))) {
        terminal_writestring("File created successfully\n");
        return 0;
    } else {
        terminal_writestring("Failed to create file\n");
        return 1;
    }
}

REGISTER_COMMAND("create", "Create a file with content", cmd_create_main)

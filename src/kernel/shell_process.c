#include "process.h"
#include "shell.h"
#include "terminal.h"
#include "keyboard.h"
#include "memory.h"
#include "ramdisk.h"
#include "fat16.h"
#include "io.h"
#include "string.h"
#include "memory_utils.h"
#include "command.h"

#define MAX_COMMAND_LENGTH 100

static char command_buffer[MAX_COMMAND_LENGTH];
static size_t command_length = 0;

// Forward declarations
static void shell_execute_command(const char* input);

// Find and execute command using the new command system
static void shell_execute_command(const char* input) {
    // Skip leading whitespace
    while (*input == ' ' || *input == '\t') {
        input++;
    }
    
    if (*input == '\0') {
        return; // Empty command
    }
    
    // Parse command line into argc/argv
    char** argv;
    int argc = command_parse_args(input, &argv);
    
    if (argc == 0) {
        return;
    }
    
    // Execute the command
    command_result_t result = command_execute(argv[0], argc, argv);
    
    switch (result) {
        case COMMAND_SUCCESS:
            // Command executed successfully
            break;
        case COMMAND_ERROR_NOT_FOUND:
            terminal_writestring("Unknown command: ");
            terminal_writestring(argv[0]);
            terminal_writestring("\nType 'help' for available commands.\n");
            break;
        case COMMAND_ERROR_EXEC_FAILED:
            terminal_writestring("Command execution failed\n");
            break;
        case COMMAND_ERROR_LOAD_FAILED:
            terminal_writestring("Failed to load command\n");
            break;
        case COMMAND_ERROR_INVALID_ARGS:
            terminal_writestring("Invalid command arguments\n");
            break;
    }
    
    // Free the parsed arguments
    command_free_args(argc, argv);
}

static void shell_process_handle_input(char c) {
    if (c == '\n') {
        terminal_writestring("\n");
        if (command_length > 0) {
            command_buffer[command_length] = '\0';
            shell_execute_command(command_buffer);
            command_length = 0;
        }
        terminal_writestring("> ");
    } else if (c == '\b') {
        if (command_length > 0) {
            command_length--;
            terminal_putchar('\b');
        }
    } else {
        if (command_length < MAX_COMMAND_LENGTH - 1) {
            command_buffer[command_length++] = c;
            terminal_putchar(c);
        }
    }
    command_buffer[command_length] = '\0';
}

// Main shell process function
void shell_process_main(void* args) {
    (void)args;
    
    terminal_writestring("Shell process started (PID: ");
    char pid_str[16];
    uint32_to_string(process_get_current()->pid, pid_str);
    terminal_writestring(pid_str);
    terminal_writestring(")\n");
    
    // Initialize shell
    command_register_builtins();  // Register all built-in commands
    command_length = 0;
    terminal_writestring("> ");
    
    // Shell main loop
    while (1) {
        char c = keyboard_getchar();
        shell_process_handle_input(c);
        
        // Don't yield on every keystroke - only occasionally
        // This keeps the shell responsive
    }
}

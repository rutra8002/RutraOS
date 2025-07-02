#include "shell.h"
#include "terminal.h"
#include "keyboard.h"

typedef unsigned long size_t;

#define MAX_COMMAND_LENGTH 100

static char command_buffer[MAX_COMMAND_LENGTH];
static size_t command_length = 0;

// String comparison helper function
static int shell_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void shell_init() {
    terminal_writestring("> ");
}

// Implementation of shutdown function
void shell_shutdown() {
    terminal_writestring("Shutting down system...\n");
    
    // ACPI shutdown for most systems
    __asm__ volatile("outw %0, %1" : : "a"((unsigned short)0x2000), "d"((unsigned short)0x604));
    
    // If we reach here, shutdown failed
    terminal_writestring("Shutdown failed.\n");
}

void shell_handle_input(char c) {
    if (c == '\n') {
        terminal_writestring("\n");
        if (command_length > 0) {
            command_buffer[command_length] = '\0';
            
            // Check if command is "shutdown"
            if (shell_strcmp(command_buffer, "shutdown") == 0) {
                shell_shutdown();
            } else if (shell_strcmp(command_buffer, "help") == 0) {
                terminal_writestring("Available commands:\n");
                terminal_writestring("- shutdown: Shut down the system\n");
                terminal_writestring("- help: Show this help message\n");
            } else {
                // Process other commands
                terminal_writestring("Command: ");
                terminal_writestring(command_buffer);
                terminal_writestring("\n");
            }
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
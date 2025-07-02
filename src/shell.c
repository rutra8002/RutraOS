#include "shell.h"
#include "terminal.h"
#include "keyboard.h"

typedef unsigned long size_t;

#define MAX_COMMAND_LENGTH 100

static char command_buffer[MAX_COMMAND_LENGTH];
static size_t command_length = 0;

void shell_init() {
    terminal_writestring("> ");
}

void shell_handle_input(char c) {
    if (c == '\n') {
        terminal_writestring("\n");
        if (command_length > 0) {
            // TODO: Process command
            terminal_writestring("Command: ");
            terminal_writestring(command_buffer);
            terminal_writestring("\n");
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

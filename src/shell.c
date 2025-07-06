#include "shell.h"
#include "terminal.h"
#include "keyboard.h"
#include "memory.h"
#include "ramdisk.h"
#include "fat12.h"

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

// String comparison helper function (n characters)
static int shell_strncmp(const char *s1, const char *s2, size_t n) {
    while (n > 0 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    return (n == 0) ? 0 : (*(const unsigned char*)s1 - *(const unsigned char*)s2);
}

// Find character in string
static char* shell_strchr(const char *s, int c) {
    while (*s) {
        if (*s == c) {
            return (char*)s;
        }
        s++;
    }
    return NULL;
}

// String length function
static size_t shell_strlen(const char *s) {
    size_t len = 0;
    while (*s++) {
        len++;
    }
    return len;
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
                terminal_writestring("- clear: Clear the terminal screen\n");
                terminal_writestring("- help: Show this help message\n");
                terminal_writestring("- meminfo: Show memory information\n");
                terminal_writestring("- memtest: Test memory allocation\n");
                terminal_writestring("- ramdisk: Show ramdisk information\n");
                terminal_writestring("- ls: List files in ramdisk\n");
                terminal_writestring("- cat <filename>: Read file from ramdisk\n");
                terminal_writestring("- create <filename> <content>: Create file in ramdisk\n");
            } else if (shell_strcmp(command_buffer, "clear") == 0) {
                terminal_clear();
            } else if (shell_strcmp(command_buffer, "meminfo") == 0) {
                memory_print_stats();
            } else if (shell_strcmp(command_buffer, "memtest") == 0) {
                terminal_writestring("Testing memory allocation...\n");
                
                // Test 1: Basic allocation
                void* ptr1 = kmalloc(256);
                if (ptr1) {
                    terminal_writestring("Allocated 256 bytes successfully\n");
                } else {
                    terminal_writestring("Failed to allocate 256 bytes\n");
                }
                
                // Test 2: Multiple allocations
                void* ptr2 = kmalloc(512);
                void* ptr3 = kmalloc(1024);
                
                if (ptr2 && ptr3) {
                    terminal_writestring("Multiple allocations successful\n");
                } else {
                    terminal_writestring("Multiple allocations failed\n");
                }
                
                // Test 3: Free memory
                kfree(ptr1);
                kfree(ptr2);
                kfree(ptr3);
                terminal_writestring("Memory freed successfully\n");
                
                // Test 4: Test memory utilities
                char test_buffer[100];
                memset(test_buffer, 'A', 50);
                test_buffer[50] = '\0';
                terminal_writestring("Memory utilities test: ");
                terminal_writestring(test_buffer);
                terminal_writestring("\n");
            } else if (shell_strcmp(command_buffer, "ramdisk") == 0) {
                ramdisk_print_info();
            } else if (shell_strcmp(command_buffer, "ls") == 0) {
                fat12_list_files();
            } else if (shell_strncmp(command_buffer, "cat ", 4) == 0) {
                // Read file command
                char* filename = command_buffer + 4;
                char file_buffer[4096];
                int size = fat12_read_file(filename, file_buffer, sizeof(file_buffer) - 1);
                if (size > 0) {
                    file_buffer[size] = '\0';
                    terminal_writestring("File contents:\n");
                    terminal_writestring(file_buffer);
                    terminal_writestring("\n");
                } else {
                    terminal_writestring("File not found or read error\n");
                }
            } else if (shell_strncmp(command_buffer, "create ", 7) == 0) {
                // Create file command - simple version
                char* args = command_buffer + 7;
                char* space = shell_strchr(args, ' ');
                if (space) {
                    *space = '\0';
                    char* filename = args;
                    char* content = space + 1;
                    
                    if (fat12_create_file(filename, content, shell_strlen(content))) {
                        terminal_writestring("File created successfully\n");
                    } else {
                        terminal_writestring("Failed to create file\n");
                    }
                } else {
                    terminal_writestring("Usage: create <filename> <content>\n");
                }
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
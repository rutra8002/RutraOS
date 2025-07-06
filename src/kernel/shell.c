#include "shell.h"
#include "terminal.h"
#include "keyboard.h"
#include "memory.h"
#include "ramdisk.h"
#include "fat16.h"
#include "io.h"
#include "string.h"
#include "memory_utils.h"

#define MAX_COMMAND_LENGTH 100

static char command_buffer[MAX_COMMAND_LENGTH];
static size_t command_length = 0;

// Command handler function pointer type
typedef void (*command_handler_t)(const char* args);

// Command structure
typedef struct {
    const char* name;
    command_handler_t handler;
    const char* description;
} shell_command_t;

void shell_init() {
    terminal_writestring("> ");
}

// Implementation of shutdown function
void shell_shutdown() {
    terminal_writestring("Shutting down system...\n");
    
    // Try ACPI shutdown first (works on most modern systems)
    __asm__ volatile("outw %0, %1" : : "a"((unsigned short)0x2000), "d"((unsigned short)0x604));
    
    // If ACPI fails, try QEMU/Bochs shutdown
    __asm__ volatile("outw %0, %1" : : "a"((unsigned short)0x2000), "d"((unsigned short)0xB004));
    
    // Try VirtualBox shutdown
    __asm__ volatile("outw %0, %1" : : "a"((unsigned short)0x2000), "d"((unsigned short)0x4004));
    
    // If shutdown methods fail, just halt the system
    terminal_writestring("System halted. You can safely power off now.\n");
    __asm__ volatile("cli");  // Disable interrupts
    while (1) {
        __asm__ volatile("hlt");  // Halt
    }
}

// Implementation of reboot function using triple fault
void shell_reboot() {
    terminal_writestring("Rebooting system...\n");
    
    // The nuclear option - force a triple fault
    __asm__ volatile("cli");           // Disable interrupts
    __asm__ volatile("lidt %0" : : "m"(*(short*)0)); // Load invalid IDT
    __asm__ volatile("int $0x03");     // Trigger interrupt with invalid IDT
    
    // This should never be reached
    __asm__ volatile("hlt");
}

// Command handler functions
static void cmd_shutdown(const char* args) {
    (void)args; // Mark parameter as unused
    shell_shutdown();
}

static void cmd_reboot(const char* args) {
    (void)args; // Mark parameter as unused
    shell_reboot();
}

static void cmd_help(const char* args) {
    (void)args; // Mark parameter as unused
    terminal_writestring("Available commands:\n");
    terminal_writestring("- shutdown: Shut down the system\n");
    terminal_writestring("- reboot: Restart the system\n");
    terminal_writestring("- clear: Clear the terminal screen\n");
    terminal_writestring("- help: Show this help message\n");
    terminal_writestring("- meminfo: Show memory information\n");
    terminal_writestring("- memtest: Test memory allocation\n");
    terminal_writestring("- ramdisk: Show ramdisk information\n");
    terminal_writestring("- ls: List files in ramdisk\n");
    terminal_writestring("- cat <filename>: Read file from ramdisk\n");
    terminal_writestring("- create <filename> <content>: Create file in ramdisk\n");
}

static void cmd_clear(const char* args) {
    (void)args; // Mark parameter as unused
    terminal_clear();
}

static void cmd_meminfo(const char* args) {
    (void)args; // Mark parameter as unused
    memory_print_stats();
}

static void cmd_memtest(const char* args) {
    (void)args; // Mark parameter as unused
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
}

static void cmd_ramdisk(const char* args) {
    (void)args; // Mark parameter as unused
    ramdisk_print_info();
}

static void cmd_ls(const char* args) {
    (void)args; // Mark parameter as unused
    fat16_list_files();
}

static void cmd_cat(const char* args) {
    if (!args || *args == '\0') {
        terminal_writestring("Usage: cat <filename>\n");
        return;
    }
    
    char file_buffer[4096];
    int size = fat16_read_file(args, file_buffer, sizeof(file_buffer) - 1);
    if (size > 0) {
        file_buffer[size] = '\0';
        terminal_writestring("File contents:\n");
        terminal_writestring(file_buffer);
        terminal_writestring("\n");
    } else {
        terminal_writestring("File not found or read error\n");
    }
}

static void cmd_create(const char* args) {
    if (!args || *args == '\0') {
        terminal_writestring("Usage: create <filename> <content>\n");
        return;
    }
    
    char* space = strchr(args, ' ');
    if (space) {
        // Create a copy of the filename by temporarily null-terminating
        char filename_buffer[256];
        size_t filename_len = space - args;
        if (filename_len >= sizeof(filename_buffer)) {
            terminal_writestring("Filename too long\n");
            return;
        }
        
        // Copy filename
        for (size_t i = 0; i < filename_len; i++) {
            filename_buffer[i] = args[i];
        }
        filename_buffer[filename_len] = '\0';
        
        char* content = space + 1;
        
        if (fat16_create_file(filename_buffer, content, strlen(content))) {
            terminal_writestring("File created successfully\n");
        } else {
            terminal_writestring("Failed to create file\n");
        }
    } else {
        terminal_writestring("Usage: create <filename> <content>\n");
    }
}

// Command table
static const shell_command_t commands[] = {
    {"shutdown", cmd_shutdown, "Shut down the system"},
    {"reboot", cmd_reboot, "Restart the system"},
    {"help", cmd_help, "Show this help message"},
    {"clear", cmd_clear, "Clear the terminal screen"},
    {"meminfo", cmd_meminfo, "Show memory information"},
    {"memtest", cmd_memtest, "Test memory allocation"},
    {"ramdisk", cmd_ramdisk, "Show ramdisk information"},
    {"ls", cmd_ls, "List files in ramdisk"},
    {"cat", cmd_cat, "Read file from ramdisk"},
    {"create", cmd_create, "Create file in ramdisk"},
    {NULL, NULL, NULL} // Sentinel
};

// Find and execute command
static void shell_execute_command(const char* input) {
    // Skip leading whitespace
    while (*input == ' ' || *input == '\t') {
        input++;
    }
    
    if (*input == '\0') {
        return; // Empty command
    }
    
    // Find the command name length
    const char* space = strchr(input, ' ');
    size_t cmd_len = space ? (size_t)(space - input) : strlen(input);
    
    // Look up command in table
    for (const shell_command_t* cmd = commands; cmd->name; cmd++) {
        if (strncmp(input, cmd->name, cmd_len) == 0 && 
            strlen(cmd->name) == cmd_len) {
            // Found the command, extract arguments
            const char* args = space ? space + 1 : "";
            cmd->handler(args);
            return;
        }
    }
    
    // Command not found
    terminal_writestring("Unknown command: ");
    // Print just the command name
    for (size_t i = 0; i < cmd_len; i++) {
        terminal_putchar(input[i]);
    }
    terminal_writestring("\nType 'help' for available commands.\n");
}

void shell_handle_input(char c) {
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

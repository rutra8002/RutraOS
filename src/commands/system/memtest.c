#include "command.h"
#include "terminal.h"
#include "memory.h"
#include "memory_utils.h"
#include "string.h"

static int cmd_memtest_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("memtest", "");
        terminal_writestring("Run memory allocation tests.\n");
        terminal_writestring("Tests basic allocation, multiple allocations, freeing,\n");
        terminal_writestring("and memory utility functions like memset.\n");
        return 0;
    }
    
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
    
    return 0;
}

REGISTER_COMMAND("memtest", "Test memory allocation", cmd_memtest_main)

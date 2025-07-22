#include "kernel.h"
#include "terminal.h"
#include "shell.h"
#include "keyboard.h"
#include "ramdisk.h"
#include "process.h"
#include "memory.h"

// Main kernel function - called from assembly
void kernel_main(void) {
    // Initialize the terminal
    terminal_initialize();
    
    // Set a nice color scheme
    terminal_setcolor(make_vga_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    
    // Print welcome message
    terminal_writestring("RutraOS 64-bit Kernel Started!\n");
    terminal_writestring("Kernel written in C with Process Management!\n\n");
    
    // Some example output
    terminal_setcolor(make_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("System Information:\n");
    terminal_writestring("- Architecture: x86_64\n");
    terminal_writestring("- Boot Method: GRUB Multiboot\n");
    terminal_writestring("- Memory Model: Long Mode (64-bit)\n");
    terminal_writestring("- Display: VGA Text Mode\n");
    terminal_writestring("- Process Management: Enabled\n\n");
    
    terminal_setcolor(make_vga_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("YAY!\n\n");

    // Initialize memory management
    memory_init();

    // Initialize ramdisk
    terminal_writestring("Initializing ramdisk...\n");
    if (ramdisk_init()) {
        if (ramdisk_format_fat16()) {
            terminal_writestring("Ramdisk ready!\n");
        } else {
            terminal_writestring("Failed to format ramdisk\n");
        }
    } else {
        terminal_writestring("Failed to initialize ramdisk\n");
    }

    // Initialize keyboard
    keyboard_init();

    // Initialize process management
    process_init();

    // Create kernel process (but don't run it yet)
    process_t* kernel_proc = process_create("kernel", kernel_process_main, NULL, 
                                          PROCESS_PRIORITY_KERNEL, 16384);
    if (!kernel_proc) {
        terminal_writestring("ERROR: Failed to create kernel process\n");
    }

    // Create shell process (but run it directly for now)
    process_t* shell_proc = process_create("shell", shell_process_main, NULL, 
                                         PROCESS_PRIORITY_HIGH, 16384);
    if (!shell_proc) {
        terminal_writestring("ERROR: Failed to create shell process\n");
        terminal_writestring("Falling back to monolithic mode...\n");
        
        // Fallback to old shell system
        shell_init();
        while (1) {
            char c = keyboard_getchar();
            shell_handle_input(c);
        }
    }

    terminal_writestring("Starting shell process...\n\n");

    // Set shell as current process and run it directly
    // This avoids complex context switching issues for now
    current_process = shell_proc;
    shell_proc->state = PROCESS_STATE_RUNNING;
    
    // Call the shell process directly - no context switching
    shell_process_main(NULL);

    // This should never be reached
    terminal_writestring("ERROR: Scheduler returned to kernel_main\n");
    while (1) {
        __asm__ volatile("hlt");
    }
}

#include "kernel.h"
#include "terminal.h"
#include "shell.h"
#include "keyboard.h"
#include "ramdisk.h"

// Main kernel function - called from assembly
void kernel_main(void) {
    // Initialize the terminal
    terminal_initialize();
    
    // Set a nice color scheme
    terminal_setcolor(make_vga_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    
    // Print welcome message
    terminal_writestring("RutraOS 64-bit Kernel Started!\n");
    terminal_writestring("Kernel written in C!\n\n");
    
    // Some example output
    terminal_setcolor(make_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("System Information:\n");
    terminal_writestring("- Architecture: x86_64\n");
    terminal_writestring("- Boot Method: GRUB Multiboot\n");
    terminal_writestring("- Memory Model: Long Mode (64-bit)\n");
    terminal_writestring("- Display: VGA Text Mode\n\n");
    
    terminal_setcolor(make_vga_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("YAY!\n");

    // Initialize ramdisk
    terminal_writestring("\nInitializing ramdisk...\n");
    if (ramdisk_init()) {
        if (ramdisk_format_fat16()) {
            terminal_writestring("Ramdisk ready!\n");
        } else {
            terminal_writestring("Failed to format ramdisk\n");
        }
    } else {
        terminal_writestring("Failed to initialize ramdisk\n");
    }

    keyboard_init();
    shell_init();

    // Infinite loop to keep the kernel running
    while (1) {
        char c = keyboard_getchar();
        shell_handle_input(c);
    }
}

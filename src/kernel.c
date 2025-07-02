#include "kernel.h"
#include "terminal.h"

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
    
    // Infinite loop to keep the kernel running
    while (1) {
        // In a real OS, this would be the main kernel loop
        // For now, we just halt
        __asm__ volatile ("hlt");
    }
}

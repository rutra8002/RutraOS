#include "kernel.h"
#include "terminal.h"
#include "shell.h"
#include "keyboard.h"
#include "mouse.h"
#include "ramdisk.h"
#include "process.h"
#include "memory.h"
#include "vga.h"
#include "ata.h"
#include "network.h"
#include "dns.h"
#include "http.h"

// Main kernel function - called from assembly
void kernel_main(void) {
    // Initialize the terminal
    terminal_initialize();
    
    // Detect disks
    ata_detect_disks();
    
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

    // Initialize mouse
    mouse_init();

    // Initialize VGA graphics (starts in text mode)
    vga_init();
    terminal_writestring("VGA graphics driver initialized\n");

    // Initialize networking
    network_init();
    dns_init();
    http_init();

    // Initialize process management
    process_init();

    // Create kernel process (but don't run it yet)
    // Priority is LOW so it only runs when nothing else is ready
    process_t* kernel_proc = process_create("kernel", kernel_process_main, NULL, 
                                          PROCESS_PRIORITY_LOW, 16384);
    if (!kernel_proc) {
        terminal_writestring("ERROR: Failed to create kernel process\n");
    }

    // Create shell process (but run it directly for now)
    // Priority is NORMAL so it runs before the idle loop
    process_t* shell_proc = process_create("shell", shell_process_main, NULL, 
                                         PROCESS_PRIORITY_NORMAL, 16384);
    if (!shell_proc) {
        terminal_writestring("ERROR: Failed to create shell process\n");
        terminal_writestring("System cannot continue without shell process.\n");
        terminal_writestring("System halted.\n");
        while (1) {
            __asm__ volatile("hlt");
        }
    }

    terminal_writestring("Starting shell process...\n\n");

    // Set kernel process as current process
    current_process = kernel_proc;
    kernel_proc->state = PROCESS_STATE_RUNNING;
    
    // Start scheduling - this will switch to the shell process
    terminal_writestring("Kernel: Handing over to scheduler...\n");
    process_schedule();

    // This becomes the idle loop for the kernel process
    terminal_writestring("Kernel: Entered idle loop (multitasking active)\n");
    while (1) {
        process_yield();
        // We cannot use hlt here because we don't have interrupts enabled yet
        // __asm__ volatile("hlt"); 
    }
}

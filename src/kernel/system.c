#include "shell.h"
#include "terminal.h"
#include "io.h"

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

void shell_reboot() {
    terminal_writestring("Rebooting system...\n");
    
    // The nuclear option - force a triple fault
    __asm__ volatile("cli");           // Disable interrupts
    __asm__ volatile("lidt %0" : : "m"(*(short*)0)); // Load invalid IDT
    __asm__ volatile("int $0x03");     // Trigger interrupt with invalid IDT
    
    // This should never be reached
    __asm__ volatile("hlt");
}

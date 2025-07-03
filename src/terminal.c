#include "terminal.h"

// Terminal state
static size_t terminal_row;
static size_t terminal_column;
static unsigned char terminal_color;

// Initialize the terminal
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = make_vga_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Clear the screen
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            ((volatile unsigned short*)VGA_BUFFER)[index] = make_vga_entry(' ', terminal_color);
        }
    }
}

// Set terminal color
void terminal_setcolor(unsigned char color) {
    terminal_color = color;
}

// Put a character at a specific position
void terminal_putentryat(char c, unsigned char color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    ((volatile unsigned short*)VGA_BUFFER)[index] = make_vga_entry(c, color);
}

// Update the VGA hardware cursor to match terminal_row and terminal_column
static void terminal_update_cursor(void) {
    unsigned short pos = (unsigned short)(terminal_row * VGA_WIDTH + terminal_column);
    // Send the high byte
    outb(0x3D4, 0x0E);
    outb(0x3D5, (pos >> 8) & 0xFF);
    // Send the low byte
    outb(0x3D4, 0x0F);
    outb(0x3D5, pos & 0xFF);
}

// Scroll the terminal up by one line
void terminal_scroll(void) {
    // Move all lines up by one
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            size_t src_index = (y + 1) * VGA_WIDTH + x;
            size_t dst_index = y * VGA_WIDTH + x;
            ((volatile unsigned short*)VGA_BUFFER)[dst_index] = 
                ((volatile unsigned short*)VGA_BUFFER)[src_index];
        }
    }
    
    // Clear the bottom line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        ((volatile unsigned short*)VGA_BUFFER)[index] = make_vga_entry(' ', terminal_color);
    }
    terminal_update_cursor();
}

// Put a character
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
            terminal_row = VGA_HEIGHT - 1;
        }
    } else if (c == '\b') {
        // Handle backspace
        if (terminal_column > 0) {
            terminal_column--;
            // Clear the character at the current position
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        }
    } else {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_scroll();
                terminal_row = VGA_HEIGHT - 1;
            }
        }
    }
    terminal_update_cursor();
}

// Print a string
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

// Print a null-terminated string
void terminal_writestring(const char* data) {
    size_t len = 0;
    while (data[len]) len++;  // Simple strlen implementation
    terminal_write(data, len);
}

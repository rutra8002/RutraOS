#ifndef TERMINAL_H
#define TERMINAL_H

#include "kernel.h"

// VGA text mode constants
#define VGA_BUFFER ((volatile char*)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Color codes for VGA text mode
typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
} vga_color;

// Inline utility functions
static inline unsigned char make_vga_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

static inline unsigned short make_vga_entry(char c, unsigned char color) {
    return (unsigned short)c | (unsigned short)color << 8;
}

// Write a byte to the specified port
static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Function declarations
void terminal_initialize(void);
void terminal_setcolor(unsigned char color);
void terminal_putentryat(char c, unsigned char color, size_t x, size_t y);
void terminal_scroll(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);

#endif // TERMINAL_H

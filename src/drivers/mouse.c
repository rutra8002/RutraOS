#include "mouse.h"
#include "io.h"
#include "vga.h"
#include "memory.h"

// Mouse state
static int mouse_x = 160;
static int mouse_y = 100;
static int mouse_buttons = 0;
static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];

// Cursor state
static uint8_t cursor_bg[100]; // 10x10 max
static int old_mouse_x = 160;
static int old_mouse_y = 100;
static int cursor_visible = 0;

// Ports
#define MOUSE_DATA_PORT 0x60
#define MOUSE_STATUS_PORT 0x64
#define MOUSE_CMD_PORT 0x64

// Commands
#define MOUSE_CMD_ENABLE_AUX 0xA8
#define MOUSE_CMD_WRITE_AUX 0xD4
#define MOUSE_DEV_SET_DEFAULTS 0xF6
#define MOUSE_DEV_ENABLE_STREAM 0xF4

// Simple pointer cursor (10x10)
// 0 = transparent, 1 = border (white), 2 = fill (black)
static const uint8_t cursor_shape[10][10] = {
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
    {1, 1, 2, 1, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 1, 0, 0, 0, 0, 0},
    {1, 1, 2, 2, 2, 1, 0, 0, 0, 0},
    {1, 0, 1, 2, 2, 2, 1, 0, 0, 0},
    {0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

static void mouse_wait(uint8_t type) {
    int timeout = 100000;
    if (type == 0) { // Data
        while (timeout--) {
            if ((inb(MOUSE_STATUS_PORT) & 1) == 1) return;
        }
    } else { // Signal
        while (timeout--) {
            if ((inb(MOUSE_STATUS_PORT) & 2) == 0) return;
        }
    }
}

static void mouse_write(uint8_t write) {
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, MOUSE_CMD_WRITE_AUX);
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, write);
}

static uint8_t mouse_read() {
    mouse_wait(0);
    return inb(MOUSE_DATA_PORT);
}

void mouse_init() {
    uint8_t status;
    
    // Enable auxiliary mouse device
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, MOUSE_CMD_ENABLE_AUX);
    
    // Enable interrupts/aux
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0x20); // Get Compaq Status
    mouse_wait(0);
    status = inb(MOUSE_DATA_PORT) | 2;
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0x60); // Set Compaq Status
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, status);
    
    // Set defaults
    mouse_write(MOUSE_DEV_SET_DEFAULTS);
    mouse_read(); // ACK
    
    // Enable streaming
    mouse_write(MOUSE_DEV_ENABLE_STREAM);
    mouse_read(); // ACK
    
    mouse_cycle = 0;
}

// Helpers for cursor drawing
static void save_cursor_bg(int x, int y) {
    for (int j = 0; j < 10; j++) {
        for (int i = 0; i < 10; i++) {
            cursor_bg[j * 10 + i] = vga_get_pixel(x + i, y + j);
        }
    }
}

static void restore_cursor_bg(int x, int y) {
    for (int j = 0; j < 10; j++) {
        for (int i = 0; i < 10; i++) {
            vga_put_pixel(x + i, y + j, cursor_bg[j * 10 + i]);
        }
    }
}

static void draw_cursor(int x, int y) {
    for (int j = 0; j < 10; j++) {
        for (int i = 0; i < 10; i++) {
            if (cursor_shape[j][i] == 1) {
                vga_put_pixel(x + i, y + j, 15); // White
            } else if (cursor_shape[j][i] == 2) {
                vga_put_pixel(x + i, y + j, 0);  // Black
            }
        }
    }
}

void mouse_handle_byte(uint8_t data) {
    switch(mouse_cycle) {
        case 0:
            if ((data & 0x08) == 0x08) {
                mouse_byte[0] = data;
                mouse_cycle++;
            }
            break;
        case 1:
            mouse_byte[1] = data;
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = data;
            mouse_cycle = 0;
            
            // Should be in graphics mode to draw
            if (!vga_state.graphics_mode) {
                cursor_visible = 0;
                return;
            }

            // Process packet
            uint8_t raw_dx = mouse_byte[1];
            uint8_t raw_dy = mouse_byte[2];
            
            int16_t dx = raw_dx;
            int16_t dy = raw_dy;
            
            // Apply sign bits from the first byte
            if (mouse_byte[0] & 0x10) dx |= 0xFF00; // X sign bit
            if (mouse_byte[0] & 0x20) dy |= 0xFF00; // Y sign bit
            
            // Restore old cursor if visible
            if (cursor_visible) {
                restore_cursor_bg(old_mouse_x, old_mouse_y);
            }
            
            mouse_x += dx;
            mouse_y -= dy; // Invert Y
            
            // Clamp
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x >= VGA_GFX_WIDTH - 10) mouse_x = VGA_GFX_WIDTH - 10;
            if (mouse_y >= VGA_GFX_HEIGHT - 10) mouse_y = VGA_GFX_HEIGHT - 10;
            
            mouse_buttons = mouse_byte[0] & 0x07;
            
            // Draw new cursor
            save_cursor_bg(mouse_x, mouse_y);
            draw_cursor(mouse_x, mouse_y);
            
            old_mouse_x = mouse_x;
            old_mouse_y = mouse_y;
            cursor_visible = 1;
            break;
    }
}

int mouse_get_x() { return mouse_x; }
int mouse_get_y() { return mouse_y; }
int mouse_get_buttons() { return mouse_buttons; }

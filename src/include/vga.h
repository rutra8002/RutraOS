#ifndef VGA_H
#define VGA_H

#include "types.h"

// Undefine text mode constants to avoid conflicts
#ifdef VGA_WIDTH
#undef VGA_WIDTH
#endif
#ifdef VGA_HEIGHT  
#undef VGA_HEIGHT
#endif

// VGA Mode 13h (320x200, 256 colors)
#define VGA_GFX_WIDTH 320
#define VGA_GFX_HEIGHT 200
#define VGA_MEMORY 0xA0000

// VGA registers
#define VGA_AC_INDEX        0x3C0
#define VGA_AC_WRITE        0x3C0
#define VGA_AC_READ         0x3C1
#define VGA_MISC_WRITE      0x3C2
#define VGA_SEQ_INDEX       0x3C4
#define VGA_SEQ_DATA        0x3C5
#define VGA_DAC_READ_INDEX  0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA        0x3C9
#define VGA_MISC_READ       0x3CC
#define VGA_GC_INDEX        0x3CE
#define VGA_GC_DATA         0x3CF
#define VGA_CRTC_INDEX      0x3D4
#define VGA_CRTC_DATA       0x3D5
#define VGA_INSTAT_READ     0x3DA

// Common colors (palette indices)
#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHT_GRAY    7
#define COLOR_DARK_GRAY     8
#define COLOR_LIGHT_BLUE    9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_CYAN    11
#define COLOR_LIGHT_RED     12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW        14
#define COLOR_WHITE         15

// VGA functions
int vga_init(void);
void vga_set_mode_13h(void);
void vga_put_pixel(int x, int y, uint8_t color);
uint8_t vga_get_pixel(int x, int y);
void vga_clear_screen(uint8_t color);
void vga_draw_line(int x1, int y1, int x2, int y2, uint8_t color);
void vga_draw_rectangle(int x, int y, int width, int height, uint8_t color);
void vga_draw_filled_rectangle(int x, int y, int width, int height, uint8_t color);
void vga_draw_circle(int cx, int cy, int radius, uint8_t color);
void vga_set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void vga_draw_char(int x, int y, char c, uint8_t fg_color, uint8_t bg_color);
void vga_draw_string(int x, int y, const char* str, uint8_t fg_color, uint8_t bg_color);

// Graphics mode state
typedef struct {
    int graphics_mode;
    int width;
    int height;
    uint8_t* framebuffer;
} vga_state_t;

extern vga_state_t vga_state;

#endif // VGA_H

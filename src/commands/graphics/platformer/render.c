#include "game.h"
#include "vga.h"
#include "rtc.h"
#include "string.h"

void draw_game(Player* p) {
    // Clear screen (naive: clear everything. Optimized: clear only moving parts)
    // For 320x200, clearing whole screen might be slowish but acceptable.
    // Let's try clearing background with a solid color
    vga_clear_screen(COLOR_BLACK);

    // Draw platforms
    for (int i = 0; i < platform_count; i++) {
        vga_draw_filled_rectangle(platforms[i].x, platforms[i].y, platforms[i].w, platforms[i].h, platforms[i].color);
    }

    // Draw player
    vga_draw_filled_rectangle(p->x, p->y, p->width, p->height, p->color);

    // Draw time
    rtc_time_t t = rtc_get_time();
    char time_str[32];
    char buf[10];
    
    time_str[0] = '\0';
    uint32_to_string_padded(t.hour, buf, 2, '0');
    strcat(time_str, buf);
    strcat(time_str, ":");
    
    uint32_to_string_padded(t.minute, buf, 2, '0');
    strcat(time_str, buf);
    strcat(time_str, ":");
    
    uint32_to_string_padded(t.second, buf, 2, '0');
    strcat(time_str, buf);

    vga_draw_string(240, 10, time_str, COLOR_WHITE, COLOR_BLACK);
    vga_draw_string(10, 10, "ESC to Quit", COLOR_WHITE, COLOR_BLACK);
}

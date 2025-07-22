#include "command.h"
#include "terminal.h"
#include "vga.h"
#include "string.h"

static int parse_int(const char* str) {
    int result = 0;
    int sign = 1;
    
    if (*str == '-') {
        sign = -1;
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

static int cmd_draw_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("draw", "<shape> [params...]");
        terminal_writestring("Draw shapes in graphics mode:\n");
        terminal_writestring("  draw pixel <x> <y> <color>          - Draw a pixel\n");
        terminal_writestring("  draw line <x1> <y1> <x2> <y2> <color> - Draw a line\n");
        terminal_writestring("  draw rect <x> <y> <w> <h> <color>    - Draw rectangle\n");
        terminal_writestring("  draw fill <x> <y> <w> <h> <color>    - Draw filled rectangle\n");
        terminal_writestring("  draw circle <cx> <cy> <radius> <color> - Draw circle\n");
        terminal_writestring("  draw clear <color>                   - Clear screen\n");
        terminal_writestring("  draw text <x> <y> <fg> <bg> <text>   - Draw text\n");
        terminal_writestring("\nColors: 0-15 (0=black, 1=blue, 2=green, 4=red, 15=white)\n");
        return 0;
    }
    
    if (!vga_state.graphics_mode) {
        terminal_writestring("Error: Not in graphics mode. Run 'gfx 13h' first.\n");
        return 1;
    }
    
    if (argc < 2) {
        terminal_writestring("Usage: draw <shape> [params...]\n");
        return 1;
    }
    
    if (strcmp(argv[1], "pixel") == 0) {
        if (argc < 5) {
            terminal_writestring("Usage: draw pixel <x> <y> <color>\n");
            return 1;
        }
        int x = parse_int(argv[2]);
        int y = parse_int(argv[3]);
        int color = parse_int(argv[4]);
        vga_put_pixel(x, y, color);
        return 0;
    }
    
    if (strcmp(argv[1], "line") == 0) {
        if (argc < 7) {
            terminal_writestring("Usage: draw line <x1> <y1> <x2> <y2> <color>\n");
            return 1;
        }
        int x1 = parse_int(argv[2]);
        int y1 = parse_int(argv[3]);
        int x2 = parse_int(argv[4]);
        int y2 = parse_int(argv[5]);
        int color = parse_int(argv[6]);
        vga_draw_line(x1, y1, x2, y2, color);
        return 0;
    }
    
    if (strcmp(argv[1], "rect") == 0) {
        if (argc < 7) {
            terminal_writestring("Usage: draw rect <x> <y> <width> <height> <color>\n");
            return 1;
        }
        int x = parse_int(argv[2]);
        int y = parse_int(argv[3]);
        int w = parse_int(argv[4]);
        int h = parse_int(argv[5]);
        int color = parse_int(argv[6]);
        vga_draw_rectangle(x, y, w, h, color);
        return 0;
    }
    
    if (strcmp(argv[1], "fill") == 0) {
        if (argc < 7) {
            terminal_writestring("Usage: draw fill <x> <y> <width> <height> <color>\n");
            return 1;
        }
        int x = parse_int(argv[2]);
        int y = parse_int(argv[3]);
        int w = parse_int(argv[4]);
        int h = parse_int(argv[5]);
        int color = parse_int(argv[6]);
        vga_draw_filled_rectangle(x, y, w, h, color);
        return 0;
    }
    
    if (strcmp(argv[1], "circle") == 0) {
        if (argc < 6) {
            terminal_writestring("Usage: draw circle <cx> <cy> <radius> <color>\n");
            return 1;
        }
        int cx = parse_int(argv[2]);
        int cy = parse_int(argv[3]);
        int r = parse_int(argv[4]);
        int color = parse_int(argv[5]);
        vga_draw_circle(cx, cy, r, color);
        return 0;
    }
    
    if (strcmp(argv[1], "clear") == 0) {
        if (argc < 3) {
            terminal_writestring("Usage: draw clear <color>\n");
            return 1;
        }
        int color = parse_int(argv[2]);
        vga_clear_screen(color);
        return 0;
    }
    
    if (strcmp(argv[1], "text") == 0) {
        if (argc < 7) {
            terminal_writestring("Usage: draw text <x> <y> <fg_color> <bg_color> <text>\n");
            return 1;
        }
        int x = parse_int(argv[2]);
        int y = parse_int(argv[3]);
        int fg = parse_int(argv[4]);
        int bg = parse_int(argv[5]);
        
        // Combine remaining arguments as text
        char text_buffer[256];
        text_buffer[0] = '\0';
        for (int i = 6; i < argc; i++) {
            if (i > 6) strcat(text_buffer, " ");
            strcat(text_buffer, argv[i]);
        }
        
        vga_draw_string(x, y, text_buffer, fg, bg);
        return 0;
    }
    
    terminal_writestring("Unknown shape. Use 'draw --help' for available shapes.\n");
    return 1;
}

REGISTER_COMMAND("draw", "Draw shapes in graphics mode", cmd_draw_main)

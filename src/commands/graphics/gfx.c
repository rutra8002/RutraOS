#include "command.h"
#include "terminal.h"
#include "vga.h"
#include "string.h"

static int cmd_gfx_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("gfx", "<mode>");
        terminal_writestring("Switch graphics modes:\n");
        terminal_writestring("  gfx init  - Initialize VGA graphics\n");
        terminal_writestring("  gfx 13h   - Set VGA Mode 13h (320x200, 256 colors)\n");
        terminal_writestring("  gfx demo  - Show graphics demo\n");
        terminal_writestring("  gfx test  - Run graphics test\n");
        return 0;
    }
    
    if (argc < 2) {
        terminal_writestring("Usage: gfx <mode>\n");
        return 1;
    }
    
    if (strcmp(argv[1], "init") == 0) {
        vga_init();
        terminal_writestring("VGA graphics initialized\n");
        return 0;
    }
    
    if (strcmp(argv[1], "13h") == 0) {
        terminal_writestring("Switching to VGA Mode 13h (320x200)...\n");
        terminal_writestring("Note: You may need to reset to return to text mode\n");
        
        // Small delay
        for (volatile int i = 0; i < 1000000; i++);
        
        vga_set_mode_13h();
        vga_clear_screen(COLOR_BLACK);
        terminal_initialize();
        return 0;
    }
    
    if (strcmp(argv[1], "demo") == 0) {
        if (!vga_state.graphics_mode) {
            terminal_writestring("Error: Not in graphics mode. Run 'gfx 13h' first.\n");
            return 1;
        }
        
        // Clear screen to blue
        vga_clear_screen(COLOR_BLUE);
        
        // Draw some shapes
        vga_draw_filled_rectangle(50, 50, 100, 60, COLOR_RED);
        vga_draw_rectangle(200, 80, 80, 40, COLOR_YELLOW);
        vga_draw_circle(160, 100, 30, COLOR_WHITE);
        vga_draw_line(10, 10, 310, 190, COLOR_GREEN);
        vga_draw_line(10, 190, 310, 10, COLOR_CYAN);
        
        // Draw some text
        vga_draw_string(60, 60, "HELLO", COLOR_WHITE, COLOR_RED);
        vga_draw_string(10, 180, "RutraOS Graphics Demo", COLOR_YELLOW, COLOR_BLUE);
        
        return 0;
    }
    
    if (strcmp(argv[1], "test") == 0) {
        if (!vga_state.graphics_mode) {
            terminal_writestring("Error: Not in graphics mode. Run 'gfx 13h' first.\n");
            return 1;
        }
        
        // Test pattern
        for (int y = 0; y < VGA_GFX_HEIGHT; y++) {
            for (int x = 0; x < VGA_GFX_WIDTH; x++) {
                uint8_t color = ((x / 20) + (y / 20)) % 16;
                vga_put_pixel(x, y, color);
            }
        }
        
        return 0;
    }
    
    terminal_writestring("Unknown graphics mode. Use 'gfx --help' for options.\n");
    return 1;
}

REGISTER_COMMAND("gfx", "Graphics mode control", cmd_gfx_main)

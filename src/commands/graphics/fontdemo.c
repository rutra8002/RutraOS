#include "command.h"
#include "terminal.h"
#include "vga.h"
#include "string.h"

static int cmd_fontdemo_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("fontdemo", "");
        terminal_writestring("Display complete font demonstration in graphics mode.\n");
        terminal_writestring("Shows all letters, numbers, and symbols.\n");
        return 0;
    }
    
    if (!vga_state.graphics_mode) {
        terminal_writestring("Error: Not in graphics mode. Run 'gfx 13h' first.\n");
        return 1;
    }
    
    // Clear screen to dark blue
    vga_clear_screen(COLOR_BLUE);
    
    // Title
    vga_draw_string(80, 5, "RutraOS Font Demonstration", COLOR_YELLOW, COLOR_BLUE);
    
    // Uppercase letters
    vga_draw_string(10, 25, "UPPERCASE:", COLOR_WHITE, COLOR_BLUE);
    vga_draw_string(10, 35, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", COLOR_LIGHT_GREEN, COLOR_BLUE);
    
    // Lowercase letters
    vga_draw_string(10, 50, "lowercase:", COLOR_WHITE, COLOR_BLUE);
    vga_draw_string(10, 60, "abcdefghijklmnopqrstuvwxyz", COLOR_LIGHT_CYAN, COLOR_BLUE);
    
    // Numbers
    vga_draw_string(10, 75, "Numbers:  0123456789", COLOR_LIGHT_RED, COLOR_BLUE);
    
    // Symbols
    vga_draw_string(10, 90, "Symbols:  !@#$%^&*()_+-=[]{}|", COLOR_LIGHT_MAGENTA, COLOR_BLUE);
    vga_draw_string(10, 100, "          <>?,./:;\"'`~", COLOR_LIGHT_MAGENTA, COLOR_BLUE);
    
    // Sentences
    vga_draw_string(10, 120, "I use Arch btw", COLOR_YELLOW, COLOR_BLUE);
    vga_draw_string(10, 130, "1234567890!", COLOR_YELLOW, COLOR_BLUE);
    
    // Programming examples
    vga_draw_string(10, 150, "int main() {", COLOR_LIGHT_GRAY, COLOR_BLUE);
    vga_draw_string(10, 160, "    printf(\"Hello World!\");", COLOR_LIGHT_GRAY, COLOR_BLUE);
    vga_draw_string(10, 170, "    return 0;", COLOR_LIGHT_GRAY, COLOR_BLUE);
    vga_draw_string(10, 180, "}", COLOR_LIGHT_GRAY, COLOR_BLUE);
    
    return 0;
}

REGISTER_COMMAND("fontdemo", "Display complete font demonstration", cmd_fontdemo_main)

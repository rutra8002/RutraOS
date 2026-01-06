#include "command.h"
#include "terminal.h"
#include "vga.h"
#include "keyboard.h"
#include "timer.h"
#include "memory.h"
#include "memory_utils.h"
#include "game.h"

// PIT Constants for framerate
#define FPS 60
#define TICKS_PER_FRAME (PIT_FREQ / FPS)

int cmd_platformer_main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    if (!vga_state.graphics_mode) {
        terminal_writestring("Error: Not in graphics mode. Run 'gfx 13h' first.\n");
        return 1;
    }

    timer_init();

    // Allocate backbuffer for double buffering
    uint8_t* backbuffer = (uint8_t*)kmalloc(SCREEN_WIDTH * SCREEN_HEIGHT);
    if (!backbuffer) {
        terminal_writestring("Error: Failed to allocate backbuffer.\n");
        return 1;
    }
    
    // Save original framebuffer pointer (VGA memory)
    uint8_t* vga_mem = (uint8_t*)0xA0000;

    Player player;
    init_game(&player);

    int running = 1;
    int key_left = 0;
    int key_right = 0;

    while (running) {
        uint16_t start_count = timer_read_count();

        // Input handling
        if (keyboard_has_input()) {
            uint8_t scancode = keyboard_read_scancode();
            
            // Key Press (Make code)
            if (scancode == KEY_A || scancode == KEY_LEFT) {
                key_left = 1;
            }
            if (scancode == KEY_D || scancode == KEY_RIGHT) {
                key_right = 1;
            }
            if ((scancode == KEY_W || scancode == KEY_UP || scancode == KEY_SPACE) && player.on_ground) {
                player.vy = JUMP_FORCE;
            }
            if (scancode == KEY_ESC) running = 0;

            // Key Release (Break code = scancode + 0x80)
            if (scancode == (KEY_A | 0x80) || scancode == (KEY_LEFT | 0x80)) {
                key_left = 0;
            }

            if (scancode == (KEY_D | 0x80) || scancode == (KEY_RIGHT | 0x80)) {
                key_right = 0;
            }
        }

        // Update velocity based on keys
        player.vx = 0;
        if (key_left) player.vx = -MOVE_SPEED;
        if (key_right) player.vx = MOVE_SPEED;

        update_player(&player);
        
        // Draw to backbuffer
        vga_state.framebuffer = backbuffer;
        draw_game(&player);
        
        // Flip buffer (copy backbuffer to VGA memory)
        memcpy(vga_mem, backbuffer, SCREEN_WIDTH * SCREEN_HEIGHT);
        
        // Restore framebuffer pointer (optional, but good practice)
        vga_state.framebuffer = vga_mem;

        // Cap to 60 FPS
        while (1) {
            uint16_t current_count = timer_read_count();
            uint16_t diff;
            if (start_count >= current_count) {
                diff = start_count - current_count;
            } else {
                diff = start_count + (65536 - current_count);
            }
            if (diff >= TICKS_PER_FRAME) break;
        }
    }

    // Clear screen on exit
    vga_clear_screen(COLOR_BLACK);
    
    // Free backbuffer
    kfree(backbuffer);
    
    return 0;
}

REGISTER_COMMAND("platformer", "Simple platformer game", cmd_platformer_main)

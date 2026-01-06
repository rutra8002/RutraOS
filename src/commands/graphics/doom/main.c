#include "command.h"
#include "terminal.h"
#include "vga.h"
#include "keyboard.h"
#include "timer.h"
#include "memory.h"
#include "memory_utils.h"
#include "doom.h"
#include "lolek.h"
#include "string.h"  // Added for memcpy
#include "mouse.h"
#include "io.h"

// FPS Control
#define FPS 30
#define TICKS_PER_FRAME (PIT_FREQ / FPS)

// Movement speed
#define MOVE_SPEED 0.10f
#define ROT_SPEED 0.08f

int cmd_doom_main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    if (!vga_state.graphics_mode) {
        terminal_writestring("Error: Not in graphics mode. Run 'gfx 13h' first.\n");
        return 1;
    }

    timer_init();
    math_init();

    // Allocate backbuffer
    uint8_t* backbuffer = (uint8_t*)kmalloc(SCREEN_W * SCREEN_H);
    if (!backbuffer) {
        terminal_writestring("Error: Failed to allocate backbuffer.\n");
        return 1;
    }
    
    // Save original framebuffer
    uint8_t* vga_mem = (uint8_t*)0xA0000;

    // Initialize player
    player_t player;
    player.x = 22.0f;
    player.y = 12.0f;
    player.dir_x = -1.0f;
    player.dir_y = 0.0f;
    player.plane_x = 0.0f;
    player.plane_y = 0.66f;

    int running = 1;
    int key_forward = 0;
    int key_backward = 0;
    int key_left = 0;
    int key_right = 0;

    
    while (running) {
        uint16_t start_count = timer_read_count();

        // Input handling - process all pending events
        while (keyboard_has_input()) {
            // Check for mouse data
            if (inb(0x64) & 0x20) {
                 mouse_handle_byte(inb(0x60));
                 continue;
            }
            uint8_t scancode = keyboard_read_scancode();
            
            // ESC
            if (scancode == KEY_ESC) running = 0;

            // Key Press
            if (scancode == KEY_W || scancode == KEY_UP) key_forward = 1;
            if (scancode == KEY_S || scancode == KEY_DOWN) key_backward = 1;
            if (scancode == KEY_D || scancode == KEY_RIGHT) key_right = 1;
            if (scancode == KEY_A || scancode == KEY_LEFT) key_left = 1;

            // Key Release
            if (scancode == (KEY_W | 0x80) || scancode == (KEY_UP | 0x80)) key_forward = 0;
            if (scancode == (KEY_S | 0x80) || scancode == (KEY_DOWN | 0x80)) key_backward = 0;
            if (scancode == (KEY_D | 0x80) || scancode == (KEY_RIGHT | 0x80)) key_right = 0;
            if (scancode == (KEY_A | 0x80) || scancode == (KEY_LEFT | 0x80)) key_left = 0;
        }

        // Apply movement
        if (key_forward) {
            if(world_map[(int)(player.x + player.dir_x * MOVE_SPEED)][(int)player.y] == 0) 
                player.x += player.dir_x * MOVE_SPEED;
            if(world_map[(int)player.x][(int)(player.y + player.dir_y * MOVE_SPEED)] == 0) 
                player.y += player.dir_y * MOVE_SPEED;
        }
        if (key_backward) {
            if(world_map[(int)(player.x - player.dir_x * MOVE_SPEED)][(int)player.y] == 0) 
                player.x -= player.dir_x * MOVE_SPEED;
            if(world_map[(int)player.x][(int)(player.y - player.dir_y * MOVE_SPEED)] == 0) 
                player.y -= player.dir_y * MOVE_SPEED;
        }
        if (key_right) {
            float old_dir_x = player.dir_x;
            player.dir_x = player.dir_x * cos(-ROT_SPEED) - player.dir_y * sin(-ROT_SPEED);
            player.dir_y = old_dir_x * sin(-ROT_SPEED) + player.dir_y * cos(-ROT_SPEED);
            float old_plane_x = player.plane_x;
            player.plane_x = player.plane_x * cos(-ROT_SPEED) - player.plane_y * sin(-ROT_SPEED);
            player.plane_y = old_plane_x * sin(-ROT_SPEED) + player.plane_y * cos(-ROT_SPEED);
        }
        if (key_left) {
            float old_dir_x = player.dir_x;
            player.dir_x = player.dir_x * cos(ROT_SPEED) - player.dir_y * sin(ROT_SPEED);
            player.dir_y = old_dir_x * sin(ROT_SPEED) + player.dir_y * cos(ROT_SPEED);
            float old_plane_x = player.plane_x;
            player.plane_x = player.plane_x * cos(ROT_SPEED) - player.plane_y * sin(ROT_SPEED);
            player.plane_y = old_plane_x * sin(ROT_SPEED) + player.plane_y * cos(ROT_SPEED);
        }
        
        // Render
        vga_state.framebuffer = backbuffer;
        raycast_render(&player);
        
        // Flip buffer
        memcpy(vga_mem, backbuffer, SCREEN_W * SCREEN_H);
        vga_state.framebuffer = vga_mem;

        // Cap FPS
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

    // Cleanup
    vga_clear_screen(COLOR_BLACK);
    kfree(backbuffer);
    
    return 0;
}

REGISTER_COMMAND("doom", "Raycasting 3D Renderer", cmd_doom_main)

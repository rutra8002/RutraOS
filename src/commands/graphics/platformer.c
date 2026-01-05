#include "command.h"
#include "terminal.h"
#include "vga.h"
#include "keyboard.h"
#include "rtc.h"
#include "string.h"
#include "io.h"
#include "memory.h"
#include "memory_utils.h"
#include "timer.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PLAYER_SIZE 10
#define GRAVITY 1
#define JUMP_FORCE -12
#define MOVE_SPEED 2
#define GROUND_LEVEL 180

// PIT Constants
#define FPS 60
#define TICKS_PER_FRAME (PIT_FREQ / FPS)

typedef struct {
    int x, y;
    int vx, vy;
    int width, height;
    int on_ground;
    uint8_t color;
} Player;

typedef struct {
    int x, y, w, h;
    uint8_t color;
} Platform;

#define MAX_PLATFORMS 10
Platform platforms[MAX_PLATFORMS];
int platform_count = 0;

void init_game(Player* p) {
    p->x = 50;
    p->y = 100;
    p->vx = 0;
    p->vy = 0;
    p->width = PLAYER_SIZE;
    p->height = PLAYER_SIZE;
    p->on_ground = 0;
    p->color = VGA_COLOR_LIGHT_GREEN;

    platform_count = 0;
    
    // Ground
    platforms[platform_count++] = (Platform){0, GROUND_LEVEL, SCREEN_WIDTH, 20, VGA_COLOR_BROWN};
    
    // Some platforms
    platforms[platform_count++] = (Platform){100, 140, 60, 10, VGA_COLOR_LIGHT_BLUE};
    platforms[platform_count++] = (Platform){200, 110, 60, 10, VGA_COLOR_LIGHT_BLUE};
    platforms[platform_count++] = (Platform){50, 80, 40, 10, VGA_COLOR_LIGHT_BLUE};
}

int check_collision(Player* p, Platform* plat) {
    return (p->x < plat->x + plat->w &&
            p->x + p->width > plat->x &&
            p->y < plat->y + plat->h &&
            p->y + p->height > plat->y);
}

void update_player(Player* p) {
    // Apply gravity
    p->vy += GRAVITY;
    if (p->vy > 5) p->vy = 5; // Terminal velocity

    // Move X
    p->x += p->vx;
    
    // Screen bounds X
    if (p->x < 0) p->x = 0;
    if (p->x + p->width > SCREEN_WIDTH) p->x = SCREEN_WIDTH - p->width;

    // Check X collisions (simple: just push back)
    // For simplicity in this basic version, we'll just do Y collisions mostly
    // or simple AABB resolution

    // Move Y
    p->y += p->vy;

    p->on_ground = 0;

    // Check collisions with platforms
    for (int i = 0; i < platform_count; i++) {
        if (check_collision(p, &platforms[i])) {
            // If falling down
            if (p->vy > 0 && p->y + p->height - p->vy <= platforms[i].y) {
                p->y = platforms[i].y - p->height;
                p->vy = 0;
                p->on_ground = 1;
            }
            // If jumping up (hit head)
            else if (p->vy < 0 && p->y - p->vy >= platforms[i].y + platforms[i].h) {
                p->y = platforms[i].y + platforms[i].h;
                p->vy = 0;
            }
        }
    }

    // Screen bounds Y
    if (p->y > SCREEN_HEIGHT) { // Fell off world
        p->x = 50;
        p->y = 0;
        p->vy = 0;
    }
}

void draw_game(Player* p) {
    // Clear screen (naive: clear everything. Optimized: clear only moving parts)
    // For 320x200, clearing whole screen might be slowish but acceptable.
    // Let's try clearing background with a solid color
    vga_clear_screen(VGA_COLOR_BLACK);

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

    vga_draw_string(240, 10, time_str, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_draw_string(10, 10, "ESC to Quit", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

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
            if (scancode == KEY_A || scancode == KEY_LEFT) key_left = 1;
            if (scancode == KEY_D || scancode == KEY_RIGHT) key_right = 1;
            if ((scancode == KEY_W || scancode == KEY_UP || scancode == KEY_SPACE) && player.on_ground) {
                player.vy = JUMP_FORCE;
            }
            if (scancode == KEY_ESC) running = 0;

            // Key Release (Break code = scancode + 0x80)
            if (scancode == (KEY_A | 0x80) || scancode == (KEY_LEFT | 0x80)) key_left = 0;
            if (scancode == (KEY_D | 0x80) || scancode == (KEY_RIGHT | 0x80)) key_right = 0;
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
    vga_clear_screen(VGA_COLOR_BLACK);
    
    // Free backbuffer
    kfree(backbuffer);
    
    return 0;
}

const command_info_t cmd_info_cmd_platformer_main = {
    .name = "platformer",
    .description = "Simple platformer game",
    .main = cmd_platformer_main,
    .version = 1
};

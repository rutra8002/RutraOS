#include "game.h"

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
    p->color = COLOR_LIGHT_GREEN;

    platform_count = 0;
    
    // Ground
    platforms[platform_count++] = (Platform){0, GROUND_LEVEL, SCREEN_WIDTH, 20, COLOR_BROWN};
    
    // Some platforms
    platforms[platform_count++] = (Platform){100, 140, 60, 10, COLOR_LIGHT_BLUE};
    platforms[platform_count++] = (Platform){200, 110, 60, 10, COLOR_LIGHT_BLUE};
    platforms[platform_count++] = (Platform){50, 80, 40, 10, COLOR_LIGHT_BLUE};
}

int check_collision(Player* p, Platform* plat) {
    return (p->x < plat->x + plat->w &&
            p->x + p->width > plat->x &&
            p->y < plat->y + plat->h &&
            p->y + p->height > plat->y);
}

void update_player(Player* p) {
    p->vy += GRAVITY;
    if (p->vy > 5) p->vy = 5;

    p->x += p->vx;
    
    if (p->x < 0) p->x = 0;
    if (p->x + p->width > SCREEN_WIDTH) p->x = SCREEN_WIDTH - p->width;

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

    if (p->y > SCREEN_HEIGHT) { // Fell off world
        p->x = 50;
        p->y = 0;
        p->vy = 0;
    }
}

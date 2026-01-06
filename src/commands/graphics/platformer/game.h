#ifndef PLATFORMER_GAME_H
#define PLATFORMER_GAME_H

#include "types.h"
#include "vga.h"

// Game Constants
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PLAYER_SIZE 10
#define GRAVITY 1
#define JUMP_FORCE -12
#define MOVE_SPEED 2
#define GROUND_LEVEL 180

#define MAX_PLATFORMS 10

// Structs
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

// Global Game State
extern Platform platforms[MAX_PLATFORMS];
extern int platform_count;

// Functions
void init_game(Player* p);
void update_player(Player* p);
void draw_game(Player* p);
int check_collision(Player* p, Platform* plat);

#endif // PLATFORMER_GAME_H

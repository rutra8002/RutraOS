#ifndef DOOM_H
#define DOOM_H

#include "types.h"

// Map dimensions
#define MAP_WIDTH 24
#define MAP_HEIGHT 24

// Screen dimensions (using Mode 13h)
#define SCREEN_W 320
#define SCREEN_H 200

// Player structure
typedef struct {
    float x, y;      // Position
    float dir_x, dir_y; // Direction vector
    float plane_x, plane_y; // Camera plane (for FOV)
} player_t;

// Game state
extern int world_map[MAP_WIDTH][MAP_HEIGHT];

void raycast_render(player_t* player);
void init_doom_game();

#endif

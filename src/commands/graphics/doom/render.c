#include "doom.h"
#include "vga.h"
#include "lolek.h"

void raycast_render(player_t* player) {
    // Clear screen (or just ceiling/floor)
    // Draw floor and ceiling
    vga_draw_filled_rectangle(0, 0, SCREEN_W, SCREEN_H / 2, COLOR_DARK_GRAY); // Ceiling
    vga_draw_filled_rectangle(0, SCREEN_H / 2, SCREEN_W, SCREEN_H / 2, COLOR_BLACK); // Floor

    for (int x = 0; x < SCREEN_W; x++) {
        // Calculate ray position and direction
        float camera_x = 2 * x / (float)SCREEN_W - 1; // x-coordinate in camera space
        float ray_dir_x = player->dir_x + player->plane_x * camera_x;
        float ray_dir_y = player->dir_y + player->plane_y * camera_x;

        // Which box of the map we're in
        int map_x = (int)player->x;
        int map_y = (int)player->y;

        // Length of ray from current position to next x or y-side
        float side_dist_x;
        float side_dist_y;

        // Length of ray from one x or y-side to next x or y-side
        float delta_dist_x = (ray_dir_x == 0) ? 1e30 : fabs(1 / ray_dir_x);
        float delta_dist_y = (ray_dir_y == 0) ? 1e30 : fabs(1 / ray_dir_y);
        float perp_wall_dist;

        // Step direction and initial sideDist
        int step_x;
        int step_y;

        int hit = 0; // Was there a wall hit? (NS or EW)
        int side;    // 0 for NS, 1 for EW

        // Calculate step and initial side_dist
        if (ray_dir_x < 0) {
            step_x = -1;
            side_dist_x = (player->x - map_x) * delta_dist_x;
        } else {
            step_x = 1;
            side_dist_x = (map_x + 1.0f - player->x) * delta_dist_x;
        }
        
        if (ray_dir_y < 0) {
            step_y = -1;
            side_dist_y = (player->y - map_y) * delta_dist_y;
        } else {
            step_y = 1;
            side_dist_y = (map_y + 1.0f - player->y) * delta_dist_y;
        }

        // Perform DDA
        while (hit == 0) {
            // Jump to next map square, either in x-direction, or in y-direction
            if (side_dist_x < side_dist_y) {
                side_dist_x += delta_dist_x;
                map_x += step_x;
                side = 0;
            } else {
                side_dist_y += delta_dist_y;
                map_y += step_y;
                side = 1;
            }
            // Check if ray has hit a wall
            if (world_map[map_x][map_y] > 0) hit = 1;
        }

        // Calculate distance projected on camera direction
        if (side == 0) perp_wall_dist = (side_dist_x - delta_dist_x);
        else           perp_wall_dist = (side_dist_y - delta_dist_y);

        // Calculate height of line to draw on screen
        int line_height = (int)(SCREEN_H / perp_wall_dist);

        // Calculate lowest and highest pixel to fill in current stripe
        int draw_start = -line_height / 2 + SCREEN_H / 2;
        if (draw_start < 0) draw_start = 0;
        int draw_end = line_height / 2 + SCREEN_H / 2;
        if (draw_end >= SCREEN_H) draw_end = SCREEN_H - 1;

        // Choose wall color
        int color = COLOR_BLUE;
        int wall_type = world_map[map_x][map_y];
        
        switch(wall_type) {
            case 1: color = COLOR_RED; break;
            case 2: color = COLOR_GREEN; break;
            case 3: color = COLOR_BLUE; break;
            case 4: color = COLOR_WHITE; break;
            default: color = COLOR_YELLOW; break;
        }

        // Give x and y sides different brightness
        if (side == 1) {
             // Simple shading: map colors to darker variants if available
             // Since we have limited palette, maybe just shift color index if we knew the palette well.
             // For now, let's just make it a different color for testing
             if (color == COLOR_RED) color = COLOR_LIGHT_RED;
             else if (color == COLOR_GREEN) color = COLOR_LIGHT_GREEN;
             else if (color == COLOR_BLUE) color = COLOR_LIGHT_BLUE;
        }

        // Draw the pixels of the stripe as a vertical line
        vga_draw_line(x, draw_start, x, draw_end, color);
    }
}

#ifndef HARRY_H
#define HARRY_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define ASSET_FNAME_SIZE 16

#define TILE_SIZE 16
#define NUM_TILES 158

#define PLAYER_TILE 56
#define PLAYER_W 20
#define PLAYER_H 16
#define PLAYER_START_X 2
#define PLAYER_START_Y 8
#define PLAYER_MOVE 2

#define FPS 30
#define FRAME_TIME_LEN (1000.0 / FPS)
#define DISPLAY_SCALE 3

#define COLOUR_WHITE 0xff

typedef struct {
    uint8_t path[256];
    uint8_t tiles[1000];
    uint8_t padding[24];
} level_t;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint16_t px;
    uint16_t py;

    uint8_t try_right;
    uint8_t try_left;
    uint8_t try_jump;
    uint8_t right;
    uint8_t left;
    uint8_t jump;
    uint8_t jump_timer;

    uint8_t collision_point[9];
    uint8_t on_ground;
    uint8_t check_pickup_x;
    uint8_t check_pickup_y;
} player_t;

typedef struct {
    bool is_running;
    uint32_t ticks_last_frame;
    uint8_t cur_level;

    uint8_t view_x;
    uint8_t view_y;
    int8_t scroll_x;

    player_t player;
    level_t level[10];
} game_state_t;

typedef struct {
    SDL_Texture *gfx_tiles[NUM_TILES];
} game_assets_t;

int game_init(void);
int game_run(void);
int game_destroy(void);

#endif // !HARRY_H

#ifndef HARRY_H
#define HARRY_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define ASSET_FNAME_SIZE 16

#define TILE_SIZE 16
#define NUM_TILES 158

#define FPS 30
#define DISPLAY_SCALE 3

struct level_t {
    uint8_t path[256];
    uint8_t tiles[1000];
    uint8_t padding[24];
};

struct game_state_t {
    bool is_running;
    uint8_t cur_level;
    uint8_t view_x;
    uint8_t view_y;
    int8_t scroll_x;

    struct level_t level[10];
};

struct game_assets_t {
    SDL_Texture *gfx_tiles[NUM_TILES];
};

int game_init(void);
int game_run(void);
int game_init_assets(void);
void game_process_input(void);
void game_update(void);
void game_render(void);
int game_destroy(void);

#endif // !HARRY_H

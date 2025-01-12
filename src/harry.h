#ifndef HARRY_H
#define HARRY_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define FPS 30
#define FRAME_TIME_LEN (1000.0 / FPS)

#define DISPLAY_SCALE 3
#define ASSET_FNAME_SIZE 16
#define TILE_SIZE 16

#define NUM_TILES 158
#define NUM_LEVELS 10
#define NUM_enemyS 5
#define NUM_START_LIVES 3

#define DEATH_TIME 30

#define PLAYER_W 20
#define PLAYER_H 16
#define PLAYER_START_X 2
#define PLAYER_START_Y 8
#define PLAYER_MOVE 2
#define BULLET_SPEED 4
#define BULLET_W 12
#define BULLET_H 3

#define COLOUR_WHITE 0xff
#define COLOUR_BLACK 0x00

#define LEVEL_1 0
#define LEVEL_2 1
#define LEVEL_3 2
#define LEVEL_4 3
#define LEVEL_5 4
#define LEVEL_6 5
#define LEVEL_7 6
#define LEVEL_8 7
#define LEVEL_9 8
#define LEVEL_10 0

#define TILE_DOOR 2
#define TILE_JETPACK 4
#define TILE_TROPHY 10
#define TILE_GUN 20
#define TILE_DEATH 129;

#define TILE_PLAYER_STANDING 56
#define TILE_PLAYER_JUMP_LEFT 67
#define TILE_PLAYER_JUMP_RIGHT 68
#define TILE_PLAYER_BULLET_LEFT 127
#define TILE_PLAYER_BULLET_RIGHT 128
#define TILE_JETPACK_LEFT 77
#define TILE_JETPACK_RIGHT 80

#define TILE_ENEMY_SPIDER 89
#define TILE_ENEMY_PURPER 93
#define TILE_ENEMY_BULLET_LEFT 121
#define TILE_ENEMY_BULLET_RIGHT 124

#define SCORE_TROPHY 1000

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

    uint8_t score;
    uint8_t lives;
    int8_t death_timer;
    int8_t tick;

    uint8_t collision_point[9];

    uint8_t try_right;
    uint8_t try_left;
    uint8_t try_down;
    uint8_t try_jump;
    uint8_t try_fire;
    uint8_t try_jetpack;

    uint8_t right;
    uint8_t left;
    uint8_t up;
    uint8_t down;
    uint8_t jump;
    uint8_t jump_timer;
    uint8_t fire;
    uint8_t using_jetpack;

    int8_t last_dir;
    uint8_t on_ground;
    uint8_t check_pickup_x;
    uint8_t check_pickup_y;
    uint8_t check_door;
    uint8_t trophy;
    uint8_t gun;
    uint8_t jetpack;
    uint8_t jetpack_delay;

    uint16_t bullet_px;
    uint16_t bullet_py;
    int8_t bullet_dir;
} player_t;

typedef struct {
    uint8_t type;
    uint8_t path_index;
    uint8_t death_timer;

    uint8_t x;
    uint8_t y;
    uint16_t px;
    uint16_t py;
    int8_t next_px;
    int8_t next_py;
} enemy_t;

typedef struct {
    bool debug;
    bool is_running;
    uint32_t ticks_last_frame;
    uint32_t delay;
    uint8_t tick;
    uint8_t cur_level;

    uint8_t view_x;
    uint8_t view_y;
    int8_t scroll_x;

    level_t level[NUM_LEVELS];
    player_t player;
    enemy_t enemys[NUM_enemyS];
    uint16_t ebullet_px;
    uint16_t ebullet_py;
    int8_t ebullet_dir;
} game_state_t;

typedef struct {
    SDL_Texture *gfx_tiles[NUM_TILES];
} game_assets_t;

int game_init(const bool debug);
int game_run(void);
int game_destroy(void);

#endif // !HARRY_H

#ifndef HH_GAME_H
#define HH_GAME_H

#include "enemy.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define FPS 30
#define FRAME_TIME_LEN (1000.0 / FPS)

#define DISPLAY_SCALE 3
#define DATA_FNAME_SIZE 20
#define ASSET_FNAME_SIZE 23
#define TILE_SIZE 16
#define GAME_AREA_TOP 100
#define GAME_AREA_BOTTOM 10

#define NUM_TILES 158
#define NUM_LEVELS 10
#define NUM_START_LIVES 3

#define SCORE_NEW_LIFE 20000
#define SCORE_LEVEL_COMPLETION 2000

#define DEATH_DURATION 30

#define RIGHT_CAMERA_SCROLL_TRIGGER_TILE 18
#define LEFT_CAMERA_SCROLL_TRIGGER_TILE 2
#define CAMERA_SCROLL_AMOUNT 80
#define NUM_TILES_TO_SCROLL_CAMERA 15

#define PLAYER_W 20
#define PLAYER_H 16
#define PLAYER_START_X 2
#define PLAYER_START_Y 8
#define PLAYER_MOVE 2
#define BULLET_SPEED 4
#define BULLET_W 12
#define BULLET_H 3
#define JETPACK_START_FUEL 255

#define LEVEL_1 0
#define LEVEL_2 1
#define LEVEL_3 2
#define LEVEL_4 3
#define LEVEL_5 4
#define LEVEL_6 5
#define LEVEL_7 6
#define LEVEL_8 7
#define LEVEL_9 8
#define LEVEL_10 9

#define TILE_DOOR 2
#define TILE_JETPACK 4
#define TILE_TROPHY 10
#define TILE_GUN 20
#define TILE_DEATH 129
#define TILE_UI_LIFE 143
#define TILE_TREE_1 33
#define TILE_TREE_2 34
#define TILE_TREE_3 35
#define TILE_STAR 41

// Player tiles
#define NUM_TILES_PLAYER_WALKING 7
static const uint8_t TILES_PLAYER_WALKING[NUM_TILES_PLAYER_WALKING] = {53, 54, 55, 56, 57, 58, 59};
#define TILES_PLAYER_WALKING_MASK_OFFSET 7
#define TILE_PLAYER_STANDING 56
#define TILE_PLAYER_JUMP_LEFT 67
#define TILE_PLAYER_JUMP_RIGHT 68
#define TILE_PLAYER_JUMP_MASK_OFFSET 2
#define NUM_TILES_PLAYER_CLIMBING 7
static const uint8_t TILES_PLAYER_CLIMBING[NUM_TILES_PLAYER_CLIMBING] = {71, 72, 73};
#define TILES_PLAYER_CLIMBING_MASK_OFFSET 3
#define TILE_PLAYER_BULLET_LEFT 127
#define TILE_PLAYER_BULLET_RIGHT 128
#define NUM_TILES_PLAYER_JETPACK 6
static const uint8_t TILES_PLAYER_JETPACK[NUM_TILES_PLAYER_JETPACK] = {77, 78, 79, 80, 81, 82};
#define TILES_PLAYER_JETPACK_MASK_OFFSET 6
#define TILE_JETPACK_LEFT 77
#define TILE_JETPACK_RIGHT 80

// General tiles
#define NUM_TILES_DEATH 4
static const uint8_t TILES_DEATH[NUM_TILES_DEATH] = {129, 130, 131, 132};

// UI tiles
#define TILE_UI_SCORE 137
#define TILE_UI_LEVEL 136
#define TILE_UI_LIVES 135
#define TILE_UI_NUM_0 148

// Scores
#define SCORE_TROPHY 1000

static const uint8_t PLAYER_START_POS[10][2] = {
    {2, 8},
    {1, 8},
    {2, 5},
    {1, 5},
    {2, 8},
    {2, 8},
    {1, 2},
    {2, 8},
    {6, 1},
    {2, 8},
};

typedef struct {
    uint8_t path[256];
    uint8_t tiles[1000];
    uint8_t padding[24];
} level_t;

typedef struct {
    // Tile grid numbers/locations are 8bit ints. [-128, 127] as there 20x10 tiles
    int8_t x;
    int8_t y;
    // Tile pixel x,y locations are 16bit ints. [-32378, 32377] as there ??x?? pixels in the window
    int16_t px;
    int16_t py;

    uint32_t score;
    uint8_t lives;
    int8_t death_timer;
    int8_t tick;

    uint8_t collision_point[9];

    bool try_right;
    bool try_left;
    bool try_down;
    bool try_jump;
    bool try_up;
    bool try_fire;
    bool try_jetpack;

    bool right;
    bool left;
    bool up;
    bool down;
    bool climb;
    bool jump;
    bool fire;
    bool using_jetpack;
    uint8_t jump_timer;

    int8_t last_dir;
    bool on_ground;
    bool check_pickup_x;
    bool check_pickup_y;
    bool check_door;
    bool can_climb;
    bool has_trophy;
    bool has_gun;
    uint8_t jetpack_fuel;
    uint8_t jetpack_delay;

    uint16_t bullet_px;
    uint16_t bullet_py;
    int8_t bullet_dir;
} player_t;

typedef struct {
    bool debug;
    bool is_running;
    uint32_t ticks_last_frame;
    uint32_t delay;
    uint8_t tick;
    uint8_t cur_level;

    uint8_t camera_x;
    uint8_t camera_y;
    int8_t scroll_x;

    level_t level[NUM_LEVELS];
    player_t player;
    enemy_t enemies[NUM_ENEMIES];
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

#endif // !HH_GAME_H

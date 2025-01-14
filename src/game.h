#ifndef HH_GAME_H
#define HH_GAME_H

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
#define NUM_ENEMIES 5
#define NUM_START_LIVES 3

#define SCORE_NEW_LIFE 20000
#define SCORE_LEVEL_COMPLETION 2000
#define SCORE_ENEMY_KILL 300

#define DEATH_TIME 30

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

// Enemy tiles
#define NUM_TILES_ENEMIES 4
static const uint8_t TILES_ENEMY_LEVEL_TWO[NUM_TILES_ENEMIES] = {89, 90, 91, 92};
static const uint8_t TILES_ENEMY_LEVEL_THREE[NUM_TILES_ENEMIES] = {93, 94, 95, 96};
static const uint8_t TILES_ENEMY_LEVEL_FOUR[NUM_TILES_ENEMIES] = {97, 98, 99, 100};
static const uint8_t TILES_ENEMY_LEVEL_FIVE[NUM_TILES_ENEMIES] = {101, 102, 103, 104};
static const uint8_t TILES_ENEMY_LEVEL_SIX[NUM_TILES_ENEMIES] = {105, 106, 107, 108};
static const uint8_t TILES_ENEMY_LEVEL_SEVEN[NUM_TILES_ENEMIES] = {109, 110, 111, 112};
static const uint8_t TILES_ENEMY_LEVEL_EIGHT[NUM_TILES_ENEMIES] = {113, 114, 115, 116};
static const uint8_t TILES_ENEMY_LEVEL_NINE[NUM_TILES_ENEMIES] = {117, 118, 119, 120};
#define TILE_ENEMY_SPIDER 89
#define TILE_ENEMY_PURPER 93
#define TILE_ENEMY_BULLET_LEFT 121
#define TILE_ENEMY_BULLET_RIGHT 124

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

typedef struct {
    uint8_t path[256];
    uint8_t tiles[1000];
    uint8_t padding[24];
} level_t;

typedef struct {
    int8_t x;
    int8_t y;
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
    uint8_t on_ground;
    uint8_t check_pickup_x;
    uint8_t check_pickup_y;
    uint8_t check_door;
    uint8_t can_climb;
    bool has_trophy;
    uint8_t gun;
    uint8_t jetpack_fuel;
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

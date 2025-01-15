#ifndef HH_ENEMY_H
#define HH_ENEMY_H

#include <stdint.h>

#define NUM_ENEMIES 5
#define NUM_LEVELS 10 // TODO:(lukefilewalker) recursive inculde - how fix/forward declare?
#define TILE_SIZE 16  // TODO:(lukefilewalker) recursive inculde - how fix/forward declare?

#define SCORE_ENEMY_KILL 300

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
#define TILE_ENEMY_SPIDY 89
#define TILE_ENEMY_PURPER 93
#define TILE_ENEMY_STARBOY 97
#define TILE_ENEMY_DUMBELL_BRO 101
#define TILE_ENEMY_UFO 105
#define TILE_ENEMY_HAMBURGER 109
#define TILE_ENEMY_APPLER 105
#define TILE_ENEMY_BULLET_LEFT 121
#define TILE_ENEMY_BULLET_RIGHT 124

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

const enemy_t ENEMIES_START_STATE[NUM_LEVELS][NUM_ENEMIES];

#endif // HH_ENEMY_H

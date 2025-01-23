#include "enemy.h"

// clang-format off
const enemy_t ENEMIES_START_STATE[NUM_LEVELS][NUM_ENEMIES] = {
    { // Level_1 
        0,
    },
    { // Level_2 
        0,
    },
    { // Level_3
        {TILE_ENEMY_SPIDY, 44 * TILE_SIZE, TILE_SIZE },
        {TILE_ENEMY_SPIDY, 59 * TILE_SIZE, TILE_SIZE},
    },
    { // Level_4
        {TILE_ENEMY_PURPER, 32 * TILE_SIZE, 2 * TILE_SIZE},
    },
    { // Level_5
        {TILE_ENEMY_STARBOY, 15 * TILE_SIZE, 3 * TILE_SIZE},
        {TILE_ENEMY_STARBOY, 33 * TILE_SIZE, 3 * TILE_SIZE},
        {TILE_ENEMY_STARBOY, 49 * TILE_SIZE, 3 * TILE_SIZE},
    },
    { // Level_6
        {TILE_ENEMY_DUMBELL_BRO, 10 * TILE_SIZE, 8 * TILE_SIZE},
        {TILE_ENEMY_DUMBELL_BRO, 28 * TILE_SIZE, 8 * TILE_SIZE},
        {TILE_ENEMY_DUMBELL_BRO, 45 * TILE_SIZE, 2 * TILE_SIZE},
        {TILE_ENEMY_DUMBELL_BRO, 40 * TILE_SIZE, 8 * TILE_SIZE},
    },
    { // Level_7
        {TILE_ENEMY_UFO, 5 * TILE_SIZE, 2 * TILE_SIZE},
        {TILE_ENEMY_UFO, 16 * TILE_SIZE, 1 * TILE_SIZE},
        {TILE_ENEMY_UFO, 46 * TILE_SIZE, 2 * TILE_SIZE},
        {TILE_ENEMY_UFO, 56 * TILE_SIZE, 2 * TILE_SIZE},
    },
    { // Level_8
        {TILE_ENEMY_HAMBURGER, 53 * TILE_SIZE, 5 * TILE_SIZE},
        {TILE_ENEMY_HAMBURGER, 72 * TILE_SIZE, 2 * TILE_SIZE},
        {TILE_ENEMY_HAMBURGER, 84 * TILE_SIZE, 1 * TILE_SIZE},
    },
    { // Level_9
        {TILE_ENEMY_APPLER, 35 * TILE_SIZE, 8 * TILE_SIZE},
        {TILE_ENEMY_APPLER, 41 * TILE_SIZE, 8 * TILE_SIZE},
        {TILE_ENEMY_APPLER, 49 * TILE_SIZE, 8 * TILE_SIZE},
        {TILE_ENEMY_APPLER, 65 * TILE_SIZE, 8 * TILE_SIZE},
    },
    { // Level_10
        {TILE_ENEMY_APPLER, 45 * TILE_SIZE, 8 * TILE_SIZE},
        {TILE_ENEMY_APPLER, 51 * TILE_SIZE, 2 * TILE_SIZE},
        {TILE_ENEMY_APPLER, 65 * TILE_SIZE, 3 * TILE_SIZE},
        {TILE_ENEMY_APPLER, 82 * TILE_SIZE, 5 * TILE_SIZE},
    },
};
// clang-format on

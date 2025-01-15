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
        (enemy_t){
            .type = TILE_ENEMY_SPIDY,
            .px = 44 * TILE_SIZE,
            .py = 4 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_SPIDY,
            .px = 59 * TILE_SIZE,
            .py = 4 * TILE_SIZE,
        },
    },
    { // Level_4
        (enemy_t){
            .type = TILE_ENEMY_PURPER,
            .px = 32 * TILE_SIZE,
            .py = 2 * TILE_SIZE,
        },
    },
    { // Level_5
        (enemy_t){
            .type = TILE_ENEMY_STARBOY,
            .px = 15 * TILE_SIZE,
            .py = 3 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_STARBOY,
            .px = 33 * TILE_SIZE,
            .py = 3 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_STARBOY,
            .px = 49 * TILE_SIZE,
            .py = 3 * TILE_SIZE,
        },
    },
    { // Level_6
        (enemy_t){
            .type = TILE_ENEMY_DUMBELL_BRO,
            .px = 10 * TILE_SIZE,
            .py = 8 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_DUMBELL_BRO,
            .px = 28 * TILE_SIZE,
            .py = 8 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_DUMBELL_BRO,
            .px = 45 * TILE_SIZE,
            .py = 2 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_DUMBELL_BRO,
            .px = 40 * TILE_SIZE,
            .py = 8 * TILE_SIZE,
        },
    },
    { // Level_7
        (enemy_t){
            .type = TILE_ENEMY_UFO,
            .px = 5 * TILE_SIZE,
            .py = 2 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_UFO,
            .px = 16 * TILE_SIZE,
            .py = 1 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_UFO,
            .px = 46 * TILE_SIZE,
            .py = 2 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_UFO,
            .px = 56 * TILE_SIZE,
            .py = 2 * TILE_SIZE,
        },
    },
    { // Level_8
        (enemy_t){
            .type = TILE_ENEMY_HAMBURGER,
            .px = 53 * TILE_SIZE,
            .py = 5 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_HAMBURGER,
            .px = 72 * TILE_SIZE,
            .py = 2 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_HAMBURGER,
            .px = 84 * TILE_SIZE,
            .py = 1 * TILE_SIZE,
        },
    },
    { // Level_9
        (enemy_t){
            .type = TILE_ENEMY_APPLER,
            .px = 35 * TILE_SIZE,
            .py = 8 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_APPLER,
            .px = 41 * TILE_SIZE,
            .py = 8 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_APPLER,
            .px = 49 * TILE_SIZE,
            .py = 8 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_APPLER,
            .px = 65 * TILE_SIZE,
            .py = 8 * TILE_SIZE,
        },
    },
    { // Level_10
        (enemy_t){
            .type = TILE_ENEMY_APPLER,
            .px = 45 * TILE_SIZE,
            .py = 8 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_APPLER,
            .px = 51 * TILE_SIZE,
            .py = 2 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_APPLER,
            .px = 65 * TILE_SIZE,
            .py = 3 * TILE_SIZE,
        },
        (enemy_t){
            .type = TILE_ENEMY_APPLER,
            .px = 82 * TILE_SIZE,
            .py = 5 * TILE_SIZE,
        },
    },
};
// clang-format on

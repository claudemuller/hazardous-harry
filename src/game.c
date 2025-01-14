#include "game.h"
#include "error.h"
#include "log.h"
#include "utils.h"
#include <SDL2/SDL_ttf.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO:(lukefilewalker) General TODOs
// - look at naming of things e.g. trophy -> has_trophy
// - look at combining or splitting up func names to make more succinct as well as readible
// - make sure func names are clear

// BUG:(lukefilewalker) animation frames of player seem to be wrong sometimes
// BUG:(lukefilewalker) collision doesn't always work very well e.g. player gets stuck on walls sometimes
// BUG:(lukefilewalker) jetpack doesn't count down

static game_state_t *game;
static game_assets_t *assets;
static SDL_Window *window;
static SDL_Renderer *renderer;
static TTF_Font *font;

// TODO:(lukefilewalker): make this better :( i.e. game debug funcs or encapsulate this or something
#define MAX_DEBUG_MESSAGES 20
static uint8_t num_debug_msgs;
static char debug_msgs[MAX_DEBUG_MESSAGES][1000];

static int init_assets(void);

static bool is_player_tile(uint8_t tile);
static bool is_enemy_tile(uint8_t tile);
static void check_collisions(void);
static void process_input(void);
static void update(float dt);
static void scroll_screen(void);
static void update_level(void);
static void start_level(void);
static void restart_level(void);
static void update_pbullet(void);
static void update_ebullet(void);
static void verify_input(void);
static void move_player(float dt);
static void move_enemies(float dt);
static void pickup_item(uint8_t, uint8_t);
static void add_score(uint16_t new_score);
static void clear_input(void);
static uint8_t update_frame(uint8_t tile, uint8_t salt);

static void render(void);
static void render_world(void);
static void render_player(void);
static void render_enemies(void);
// TODO:(lukefilewalker) combine these into render funcs?
static void render_player_bullet(void);
static void render_enemies_bullet(void);
static void render_ui(void);
static void render_debug_ui(void);

static uint8_t is_clear(uint16_t px, uint16_t py, uint8_t is_player);
static uint8_t is_visible(uint16_t px);
static void add_debug_msg(char *format, char *msg);

int game_init(const bool debug)
{
    LOG_INFO("game_init", "initialising game");

    char *version = "0.1.0";
    add_debug_msg("version: %s", version);

    LOG_INFO("game_init", "allocating memory for game state");

    game = malloc(sizeof(game_state_t));
    if (!game) {
        return err_fatal(ERR_ALLOC, "game state");
    }

    // Init game state
    memset(game, 0, sizeof(game_state_t));
    game->debug = debug;
    game->ticks_last_frame = SDL_GetTicks();
    game->cur_level = LEVEL_1;

    // Init player
    game->player.on_ground = 1;
    game->player.lives = NUM_START_LIVES;

    LOG_INFO("game_init", "loading levels");

    FILE *fd_level;
    char fname[DATA_FNAME_SIZE];
    char file_num[4];

    for (int i = 0; i < NUM_LEVELS; i++) {
        fname[0] = '\0';
        char *basename = "res/data/level";
        strncat(fname, basename, strlen(basename));
        sprintf(&file_num[0], "%u", i);
        strncat(fname, file_num, strlen(file_num));
        strncat(fname, ".dat", strlen(".dat") + 1);

        fd_level = fopen(fname, "rb");
        if (!fd_level) {
            return err_fatal(ERR_OPENING_FILE, fname);
        }

        for (size_t j = 0; j < sizeof(game->level[i].path); j++) {
            game->level[i].path[j] = fgetc(fd_level);
        }
        for (size_t j = 0; j < sizeof(game->level[i].tiles); j++) {
            game->level[i].tiles[j] = fgetc(fd_level);
        }
        for (size_t j = 0; j < sizeof(game->level[i].padding); j++) {
            game->level[i].padding[j] = fgetc(fd_level);
        }

        fclose(fd_level);
    }

    LOG_INFO("game_init", "initialising SDL");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return err_fatal(ERR_SDL_INIT, SDL_GetError());
    }

    if (TTF_Init() == -1) {
        return err_fatal(ERR_SDL_TTF, SDL_GetError());
    }

    if (SDL_CreateWindowAndRenderer(320 * DISPLAY_SCALE, 200 * DISPLAY_SCALE, 0, &window, &renderer) != 0) {
        return err_fatal(ERR_SDL_CREATE_WIN_RENDER, SDL_GetError());
    }

    SDL_RenderSetScale(renderer, DISPLAY_SCALE, DISPLAY_SCALE);

    font = TTF_OpenFont("./res/fonts/Roboto-Medium.ttf", 16);
    if (!font) {
        return err_fatal(ERR_SDL_TTF_LOAD_FONT, SDL_GetError());
    }

    LOG_INFO("game_init", "allocating memory for assets");

    assets = malloc(sizeof(game_assets_t));
    if (!assets) {
        return err_fatal(ERR_ALLOC, "game assets");
    }
    int err = init_assets();
    if (err != SUCCESS) {
        return err_fatal(err, NULL);
    }

    game->is_running = true;

    return SUCCESS;
}

int game_run(void)
{
    LOG_INFO("game_run", "running game");

    uint32_t timer_start = 0, timer_end = 0, delay = 0;

    start_level();

    while (game->is_running) {
        timer_start = SDL_GetTicks();

        process_input();

        // uint32_t current_ticks = SDL_GetTicks();
        // int time_to_wait = FRAME_TIME_LEN - (current_ticks - game->ticks_last_frame);
        //
        // if (time_to_wait > 0 && time_to_wait <= FRAME_TIME_LEN) {
        //     SDL_Delay(time_to_wait);
        // }
        //
        // float dt = (current_ticks - game->ticks_last_frame) / 1000.0f;
        // game->ticks_last_frame = current_ticks;

        check_collisions();
        pickup_item(game->player.check_pickup_x, game->player.check_pickup_y);
        update(1);
        render();

        timer_end = SDL_GetTicks();

        delay = FPS - (timer_end - timer_start);
        delay = delay > 33 ? 0 : delay;
        game->delay = delay;

        // char fps_msg[256];
        // sprintf(fps_msg, "timer_start: %d - timer_end: %d", timer_start, timer_end);
        // log_info("game_run", fps_msg);

        SDL_Delay(delay);
    }

    return SUCCESS;
}

int game_destroy(void)
{
    LOG_INFO("game_destroy", "cleaning up");

    free(assets);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(game);

    return SUCCESS;
}

static int init_assets(void)
{
    LOG_INFO("init_assets", "entered");

    char *basename = "res/assets/tile";
    char fname[ASSET_FNAME_SIZE] = {0};
    char file_num[4] = {0};
    char mname[ASSET_FNAME_SIZE] = {0};
    char mask_num[4] = {0};
    SDL_Surface *surface = NULL;
    SDL_Surface *mask_surface = NULL;
    uint8_t mask_offset = 0;
    uint8_t *player_pixels = 0;
    uint8_t *mask_pixels = 0;

    for (size_t i = 0; i < NUM_TILES; i++) {
        fname[0] = '\0';
        strncat(fname, basename, strlen(basename));
        sprintf(&file_num[0], "%u", (uint8_t)i);
        strncat(fname, file_num, strlen(file_num));
        strncat(fname, ".bmp", strlen(".bmp") + 1);

        // Load player tiles
        if (is_player_tile(i)) {
            // Apply mask to walking tiles
            if (in_array(TILES_PLAYER_WALKING, 53, NUM_TILES_PLAYER_WALKING)) {
                mask_offset = TILES_PLAYER_WALKING_MASK_OFFSET;
            }

            // Apply mask to climbing tiles
            if (in_array(TILES_PLAYER_CLIMBING, i, NUM_TILES_PLAYER_CLIMBING)) {
                mask_offset = TILES_PLAYER_CLIMBING_MASK_OFFSET;
            }

            // Apply mask to jumping left and right
            if (i == TILE_PLAYER_JUMP_LEFT || i == TILE_PLAYER_JUMP_RIGHT) {
                mask_offset = TILE_PLAYER_JUMP_MASK_OFFSET;
            }

            // Apply mask to jetpack tiles
            if (in_array(TILES_PLAYER_JETPACK, i, NUM_TILES_PLAYER_JETPACK)) {
                mask_offset = TILES_PLAYER_JETPACK_MASK_OFFSET;
            }

            surface = SDL_LoadBMP(fname);
            if (!surface) {
                return err_fatal(ERR_SDL_LOADING_BMP, fname);
            }
            player_pixels = (uint8_t *)surface->pixels;

            mname[0] = '\0';
            strncat(mname, basename, strlen(basename));
            sprintf(&mask_num[0], "%u", (uint8_t)i + mask_offset);
            strncat(mname, mask_num, strlen(mask_num));
            strncat(mname, ".bmp", strlen(".bmp") + 1);

            mask_surface = SDL_LoadBMP(mname);
            if (!mask_surface) {
                return err_fatal(ERR_SDL_LOADING_BMP, mname);
            }
            mask_pixels = (uint8_t *)mask_surface->pixels;

            // Go through tile and make pixels white where they aren't black
            // ---pitch---
            // ··········· |
            // ··········· h
            // ··········· |
            for (size_t j = 0; j < (uint64_t)mask_surface->pitch * mask_surface->h; j++) {
                player_pixels[j] = mask_pixels[j] ? COLOUR_WHITE : player_pixels[j];
            }
            SDL_SetColorKey(surface, 1, SDL_MapRGB(surface->format, COLOUR_WHITE, COLOUR_WHITE, COLOUR_WHITE));
            assets->gfx_tiles[i] = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_FreeSurface(surface);
            SDL_FreeSurface(mask_surface);

            continue;
        }

        // Load all the other tiles
        surface = SDL_LoadBMP(fname);
        if (!surface) {
            return err_fatal(ERR_SDL_LOADING_BMP, fname);
        }

        // Colour key enemy and death tiles
        if (is_enemy_tile(i) || in_array(TILES_DEATH, i, NUM_TILES_DEATH)) {
            SDL_SetColorKey(surface, 1, SDL_MapRGB(surface->format, COLOUR_BLACK, COLOUR_BLACK, COLOUR_BLACK));
        }

        assets->gfx_tiles[i] = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_FreeSurface(surface);
    }

    return SUCCESS;
}

static bool is_player_tile(uint8_t tile)
{
    return in_array(TILES_PLAYER_WALKING, tile, NUM_TILES_PLAYER_WALKING) || tile == TILE_PLAYER_JUMP_LEFT ||
           tile == TILE_PLAYER_JUMP_RIGHT || in_array(TILES_PLAYER_CLIMBING, tile, NUM_TILES_PLAYER_CLIMBING) ||
           in_array(TILES_PLAYER_JETPACK, tile, NUM_TILES_PLAYER_JETPACK);
}

static bool is_enemy_tile(uint8_t tile)
{
    return in_array(TILES_ENEMY_ONE, tile, NUM_TILES_ENEMIES) || in_array(TILES_ENEMY_TWO, tile, NUM_TILES_ENEMIES) ||
           in_array(TILES_ENEMY_THREE, tile, NUM_TILES_ENEMIES) ||
           in_array(TILES_ENEMY_FOUR, tile, NUM_TILES_ENEMIES) || in_array(TILES_ENEMY_FIVE, tile, NUM_TILES_ENEMIES) ||
           in_array(TILES_ENEMY_SIX, tile, NUM_TILES_ENEMIES) || in_array(TILES_ENEMY_SEVEN, tile, NUM_TILES_ENEMIES) ||
           in_array(TILES_ENEMY_EIGHT, tile, NUM_TILES_ENEMIES);
}

static void check_collisions(void)
{
    // TODO:(lukefilewalker): change to is_colliding
    game->player.collision_point[0] = is_clear(game->player.px + 4, game->player.py - 1, 1);
    game->player.collision_point[1] = is_clear(game->player.px + 10, game->player.py - 1, 1);
    game->player.collision_point[2] = is_clear(game->player.px + 11, game->player.py + 4, 1);
    game->player.collision_point[3] = is_clear(game->player.px + 11, game->player.py + 12, 1);
    game->player.collision_point[4] = is_clear(game->player.px + 10, game->player.py + 16, 1);
    game->player.collision_point[5] = is_clear(game->player.px + 4, game->player.py + 16, 1);
    game->player.collision_point[6] = is_clear(game->player.px + 3, game->player.py + 12, 1);
    game->player.collision_point[7] = is_clear(game->player.px + 3, game->player.py + 4, 1);
    game->player.on_ground =
        ((!game->player.collision_point[4] && !game->player.collision_point[5]) || game->player.climb);

    uint8_t grid_x = (game->player.px + 6) / TILE_SIZE;
    uint8_t grid_y = (game->player.py + 8) / TILE_SIZE;
    uint8_t type;

    if (grid_x < 100 && grid_y < 10) {
        type = game->level[game->cur_level].tiles[grid_y * 100 + grid_x];
    } else {
        type = 0;
    }

    if ((type >= TILE_TREE_1 && type <= TILE_TREE_3) || type == TILE_STAR) {
        game->player.can_climb = 1;
    } else {
        game->player.can_climb = 0;
        game->player.climb = 0;
    }
}

static void process_input(void)
{
    SDL_PumpEvents();
    const uint8_t *keystate = SDL_GetKeyboardState(NULL);

    if (keystate[SDL_SCANCODE_RIGHT]) {
        game->player.try_right = 1;
    }
    if (keystate[SDL_SCANCODE_LEFT]) {
        game->player.try_left = 1;
    }
    if (keystate[SDL_SCANCODE_UP]) {
        game->player.try_up = 1;
    }
    if (keystate[SDL_SCANCODE_SPACE]) {
        game->player.try_jump = 1;
    }
    if (keystate[SDL_SCANCODE_DOWN]) {
        game->player.try_down = 1;
    }
    if (keystate[SDL_SCANCODE_LCTRL]) {
        game->player.try_fire = 1;
    }
    if (keystate[SDL_SCANCODE_LALT]) {
        game->player.try_jetpack = 1;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT: {
            game->is_running = false;
        } break;

        case SDL_KEYDOWN: {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                game->is_running = false;
            }

            //     if (event.key.keysym.sym == SDLK_RIGHT) {
            //         game->scroll_x = 15;
            //     }
            //
            //     if (event.key.keysym.sym == SDLK_LEFT) {
            //         game->scroll_x = -15;
            //     }
            //
            //     if (event.key.keysym.sym == SDLK_DOWN) {
            //         game->cur_level++;
            //     }
            //
            //     if (event.key.keysym.sym == SDLK_UP) {
            //         game->cur_level--;
            //     }
        } break;
        }
    }
}

static void update(float dt)
{
    update_pbullet();
    update_ebullet();
    verify_input();
    move_player(dt);
    move_enemies(dt);
    scroll_screen();
    update_level();
    clear_input();
}

static void render(void)
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    render_world();
    render_player();
    render_enemies();
    render_player_bullet();
    render_enemies_bullet();
    render_ui();

    if (game->debug) {
        SDL_RenderSetScale(renderer, 1, 1);
        render_debug_ui();
        SDL_RenderSetScale(renderer, DISPLAY_SCALE, DISPLAY_SCALE);
    }

    SDL_RenderPresent(renderer);
}

static void scroll_screen(void)
{
    if (game->player.x - game->view_x >= 18) {
        game->scroll_x = 15;
    }
    if (game->player.x - game->view_x < 2) {
        game->scroll_x = -15;
    }

    if (game->scroll_x > 0) {
        if (game->view_x == 80) {
            game->scroll_x = 0;
        } else {
            game->view_x++;
            game->scroll_x--;
        }
    }
    if (game->scroll_x < 0) {
        if (game->view_x == 0) {
            game->scroll_x = 0;
        } else {
            game->view_x--;
            game->scroll_x++;
        }
    }
}

static void update_level(void)
{
    game->tick++;

    if (game->player.jetpack_delay) {
        // game->player.jetpack_delay--;
    }

    // Jetpacks burn fuel when in use
    if (game->player.using_jetpack) {
        game->player.jetpack--;
        if (!game->player.jetpack) {
            game->player.using_jetpack = 0;
        }
    }

    if (game->player.check_door) {
        if (game->player.trophy) {
            add_score(SCORE_LEVEL_COMPLETION);

            if (game->cur_level < 9) {
                game->cur_level++;
                start_level();
            } else {
                printf("Winner, winner, chicken dinner - your score was %u!\n", game->player.score);
                game->is_running = false;
            }
        } else {
            game->player.check_door = 0;
        }
    }

    if (game->player.death_timer) {
        game->player.death_timer--;
        // If player died
        if (!game->player.death_timer) {
            // If player has lives remaining
            if (game->player.lives) {
                game->player.lives--;
                // TODO:(lukefilewalker): does this have to be its own func? i.e. start_level(cur_level)
                restart_level();
            } else {
                game->is_running = false;
            }
        }
    }

    for (size_t i = 0; i < NUM_ENEMIES; i++) {
        if (game->enemies[i].death_timer) {
            game->enemies[i].death_timer--;
            // Enemy has died
            if (!game->enemies[i].death_timer) {
                game->enemies[i].type = 0;
            }
        } else {
            if (game->enemies[i].type) {
                // If player and enemy collide, everyone dies
                if (game->enemies[i].x == game->player.x && game->enemies[i].y == game->player.y) {
                    game->player.death_timer = DEATH_TIME;
                    game->enemies[i].death_timer = DEATH_TIME;
                }
            }
        }
    }
}

static void start_level(void)
{
    restart_level();

    for (int i = 0; i < NUM_ENEMIES; i++) {
        game->enemies[i].type = 0;
    }

    switch (game->cur_level) {
    case LEVEL_3: {
        game->enemies[0].type = 89;
        game->enemies[0].px = 44 * TILE_SIZE;
        game->enemies[0].py = 4 * TILE_SIZE;

        game->enemies[1].type = 89;
        game->enemies[1].px = 59 * TILE_SIZE;
        game->enemies[1].py = 4 * TILE_SIZE;
    } break;
    case LEVEL_4: {
        game->enemies[0].type = 93;
        game->enemies[0].px = 32 * TILE_SIZE;
        game->enemies[0].py = 2 * TILE_SIZE;
    } break;
    case LEVEL_5: {
        game->enemies[0].type = 97;
        game->enemies[0].px = 15 * TILE_SIZE;
        game->enemies[0].py = 3 * TILE_SIZE;
        game->enemies[1].type = 97;
        game->enemies[1].px = 33 * TILE_SIZE;
        game->enemies[1].py = 3 * TILE_SIZE;
        game->enemies[2].type = 97;
        game->enemies[2].px = 49 * TILE_SIZE;
        game->enemies[2].py = 3 * TILE_SIZE;
    } break;
    case LEVEL_6: {
        game->enemies[0].type = 101;
        game->enemies[0].px = 10 * TILE_SIZE;
        game->enemies[0].py = 8 * TILE_SIZE;
        game->enemies[1].type = 101;
        game->enemies[1].px = 28 * TILE_SIZE;
        game->enemies[1].py = 8 * TILE_SIZE;
        game->enemies[2].type = 101;
        game->enemies[2].px = 45 * TILE_SIZE;
        game->enemies[2].py = 2 * TILE_SIZE;
        game->enemies[3].type = 101;
        game->enemies[3].px = 40 * TILE_SIZE;
        game->enemies[3].py = 8 * TILE_SIZE;
    } break;
    case LEVEL_7: {
        game->enemies[0].type = 105;
        game->enemies[0].px = 5 * TILE_SIZE;
        game->enemies[0].py = 2 * TILE_SIZE;
        game->enemies[1].type = 105;
        game->enemies[1].px = 16 * TILE_SIZE;
        game->enemies[1].py = 1 * TILE_SIZE;
        game->enemies[2].type = 105;
        game->enemies[2].px = 46 * TILE_SIZE;
        game->enemies[2].py = 2 * TILE_SIZE;
        game->enemies[3].type = 105;
        game->enemies[3].px = 56 * TILE_SIZE;
        game->enemies[3].py = 3 * TILE_SIZE;
    } break;
    case LEVEL_8: {
        game->enemies[0].type = 109;
        game->enemies[0].px = 53 * TILE_SIZE;
        game->enemies[0].py = 5 * TILE_SIZE;
        game->enemies[1].type = 109;
        game->enemies[1].px = 72 * TILE_SIZE;
        game->enemies[1].py = 2 * TILE_SIZE;
        game->enemies[2].type = 109;
        game->enemies[2].px = 84 * TILE_SIZE;
        game->enemies[2].py = 1 * TILE_SIZE;
    } break;
    case LEVEL_9: {
        game->enemies[0].type = 113;
        game->enemies[0].px = 35 * TILE_SIZE;
        game->enemies[0].py = 8 * TILE_SIZE;
        game->enemies[1].type = 113;
        game->enemies[1].px = 41 * TILE_SIZE;
        game->enemies[1].py = 8 * TILE_SIZE;
        game->enemies[2].type = 113;
        game->enemies[2].px = 49 * TILE_SIZE;
        game->enemies[2].py = 8 * TILE_SIZE;
        game->enemies[3].type = 113;
        game->enemies[3].px = 65 * TILE_SIZE;
        game->enemies[3].py = 8 * TILE_SIZE;
    } break;
    case LEVEL_10: {
        game->enemies[0].type = 117;
        game->enemies[0].px = 45 * TILE_SIZE;
        game->enemies[0].py = 8 * TILE_SIZE;
        game->enemies[1].type = 117;
        game->enemies[1].px = 51 * TILE_SIZE;
        game->enemies[1].py = 2 * TILE_SIZE;
        game->enemies[2].type = 117;
        game->enemies[2].px = 65 * TILE_SIZE;
        game->enemies[2].py = 3 * TILE_SIZE;
        game->enemies[3].type = 117;
        game->enemies[3].px = 82 * TILE_SIZE;
        game->enemies[3].py = 5 * TILE_SIZE;
    } break;

    default:
        break;
    }

    game->player.px = game->player.x * TILE_SIZE;
    game->player.py = game->player.y * TILE_SIZE;
    game->player.trophy = 0;
    game->player.gun = 0;
    game->player.fire = 0;
    game->player.using_jetpack = 0;
    game->player.jetpack = 0;
    game->player.death_timer = 0;
    game->player.check_door = 0;
    game->player.jump_timer = 0;
    game->view_x = 0;
    game->view_y = 0;
    game->player.last_dir = 0;
    game->player.bullet_px = 0;
    game->player.bullet_py = 0;
    game->player.bullet_dir = 0;
    game->ebullet_px = 0;
    game->ebullet_py = 0;
    game->ebullet_dir = 0;
}
static void restart_level(void)
{
    // Set player positions in level
    switch (game->cur_level) {
    case 0: {
        game->player.x = 2;
        game->player.y = 8;
    } break;

    case 1: {
        game->player.x = 1;
        game->player.y = 8;
    } break;

    case 2: {
        game->player.x = 2;
        game->player.y = 5;
    } break;

    case 3: {
        game->player.x = 1;
        game->player.y = 5;
    } break;

    case 4: {
        game->player.x = 2;
        game->player.y = 8;
    } break;

    case 5: {
        game->player.x = 2;
        game->player.y = 8;
    } break;

    case 6: {
        game->player.x = 1;
        game->player.y = 2;
    } break;

    case 7: {
        game->player.x = 2;
        game->player.y = 8;
    } break;

    case 8: {
        game->player.x = 6;
        game->player.y = 1;
    } break;

    case 9: {
        game->player.x = 2;
        game->player.y = 8;
    } break;
    }

    game->player.px = game->player.x * TILE_SIZE;
    game->player.py = game->player.y * TILE_SIZE;
}

static void update_pbullet(void)
{

    if (!game->player.bullet_px || !game->player.bullet_py) {
        return;
    }

    // If bullet hits a collidable tile, remove the bullet
    if (!is_clear(game->player.bullet_px, game->player.bullet_py, 0)) {
        game->player.bullet_px = game->player.bullet_py = 0;
    }

    uint8_t grid_x = game->player.bullet_px / TILE_SIZE;
    uint8_t grid_y = game->player.bullet_py / TILE_SIZE;

    // If bullet reaches the end of the screen, remove it
    if (grid_x - game->view_x < 1 || grid_x - game->view_x > 20) {
        game->player.bullet_px = game->player.bullet_py = 0;
    }

    if (game->player.bullet_px) {
        game->player.bullet_px += game->player.bullet_dir * BULLET_SPEED;

        for (size_t i = 0; i < NUM_ENEMIES; i++) {
            if (game->enemies[i].type) {
                uint8_t mx = game->enemies[i].x;
                uint8_t my = game->enemies[i].y;

                if ((grid_y == my || grid_y == my + 1) && (grid_x == mx || grid_x == mx + 1)) {
                    game->player.bullet_px = game->player.bullet_py = 0;
                    game->enemies[i].death_timer = DEATH_TIME;
                    add_score(SCORE_ENEMY_KILL);
                }
            }
        }
    }
}

// TODO:(lukefilewalker): combine with pullet update?
static void update_ebullet(void)
{
    if (!game->ebullet_px || !game->ebullet_py) {
        return;
    }

    // If bullet hits a collidable tile, remove it
    if (!is_clear(game->ebullet_px, game->ebullet_py, 0)) {
        game->ebullet_px = game->ebullet_py = 0;
    }

    // If bullet reaches the end of the screen, remove it
    if (!is_visible(game->ebullet_px)) {
        game->ebullet_px = game->ebullet_py = 0;
    }

    if (game->ebullet_px) {
        game->ebullet_px += game->ebullet_dir * BULLET_SPEED;

        uint8_t grid_x = game->ebullet_px / TILE_SIZE;
        uint8_t grid_y = game->ebullet_py / TILE_SIZE;

        if ((grid_y == game->player.y || grid_y == game->player.y + 1) &&
            (grid_x == game->player.x || grid_x == game->player.x + 1)) {
            game->ebullet_px = game->ebullet_py = 0;
            game->player.death_timer = DEATH_TIME;
        }
    }
}

static void verify_input(void)
{
    if (game->player.death_timer) {
        return;
    }

    if (game->player.try_right && game->player.collision_point[2] && game->player.collision_point[3]) {
        game->player.right = 1;
    }

    if (game->player.try_left && game->player.collision_point[6] && game->player.collision_point[7]) {
        game->player.left = 1;
    }

    if (game->player.try_jump && game->player.on_ground && !game->player.jump && !game->player.using_jetpack &&
        !game->player.can_climb && game->player.collision_point[0] && game->player.collision_point[1]) {
        game->player.jump = 1;
    }

    if (game->player.try_up && game->player.can_climb) {
        game->player.up = 1;
        game->player.climb = 1;
    }

    if (game->player.try_fire && game->player.gun && !game->player.bullet_px && !game->player.bullet_py) {
        game->player.fire = 1;
    }

    if (game->player.try_jetpack && game->player.jetpack && !game->player.jetpack_delay) {
        game->player.using_jetpack = !game->player.using_jetpack;
        game->player.jetpack_delay = 10;
    }

    if (game->player.try_down && (game->player.using_jetpack || game->player.climb) &&
        game->player.collision_point[4] && game->player.collision_point[5]) {
        game->player.down = 1;
    }

    if (game->player.try_jump && game->player.using_jetpack && game->player.collision_point[0] &&
        game->player.collision_point[1]) {
        game->player.up = 1;
    }
}

static void move_player(float dt)
{
    // if (game->player.death_timer) {
    //     return;
    // }

    game->player.x = game->player.px / TILE_SIZE;
    game->player.y = game->player.py / TILE_SIZE;

    if (game->player.y > 9) {
        game->player.y = 0;
        game->player.py = -16;
    }

    if (game->player.right) {
        // float px = PLAYER_MOVE; // * MUL * dt;
        game->player.px += PLAYER_MOVE;
        game->player.right = 0;
        game->player.last_dir = 1;
        game->player.tick++;
    }
    if (game->player.left) {
        // float px = PLAYER_MOVE; // * MUL * dt;
        game->player.px -= PLAYER_MOVE;
        game->player.left = 0;
        game->player.last_dir = -1;
        game->player.tick++;
    }

    if (game->player.down) {
        game->player.py += PLAYER_MOVE;
        game->player.down = 0;
    }

    if (game->player.up) {
        game->player.py -= PLAYER_MOVE;
        game->player.up = 0;
    }

    if (game->player.using_jetpack) {
        game->player.jump = 0;
        game->player.jump_timer = 0;
    }

    if (game->player.jump) {
        if (!game->player.jump_timer) {
            game->player.jump_timer = 30;
            game->player.last_dir = 0;
        }

        // TODO:(lukefilewalker): add delta time to jump
        if (game->player.collision_point[0] && game->player.collision_point[1]) {
            if (game->player.jump_timer > 16) {
                game->player.py -= PLAYER_MOVE;
            }
            if (game->player.jump_timer >= 12 && game->player.jump_timer <= 15) {
                game->player.py -= PLAYER_MOVE / 2;
            }
        }

        game->player.jump_timer--;

        if (game->player.jump_timer == 0) {
            game->player.jump = 0;
        }
    }

    // Add gravity
    if (!game->player.jump && !game->player.on_ground && !game->player.using_jetpack && !game->player.climb) {
        if (is_clear(game->player.px + 4, game->player.py + 17, 1)) {
            game->player.py += PLAYER_MOVE;
        } else {
            uint8_t not_aligned = game->player.py % TILE_SIZE;
            if (not_aligned) {
                game->player.py =
                    not_aligned < 8 ? game->player.py - not_aligned : game->player.py + TILE_SIZE - not_aligned;
            }
        }
    }

    // Firing the gun
    if (game->player.fire) {
        game->player.bullet_dir = game->player.last_dir;

        if (!game->player.bullet_dir) {
            game->player.bullet_dir = 1;
        }

        if (game->player.bullet_dir == 1) {
            game->player.bullet_px = game->player.px + 18;
        }

        if (game->player.bullet_dir == -1) {
            game->player.bullet_px = game->player.px - 8;
        }

        game->player.bullet_py = game->player.py + 8;
        game->player.fire = 0;
    }
}

static void move_enemies(float dt)
{
    for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
        enemy_t *m = &game->enemies[i];
        if (m->type && !m->death_timer) {
            // Move enemies twice as fast
            // TODO:(lukefilewalker) is there a better way to do this?
            for (int j = 0; j < 2; j++) {
                if (!m->next_px && !m->next_py) {
                    m->next_px = game->level[game->cur_level].path[m->path_index];
                    m->next_py = game->level[game->cur_level].path[m->path_index + 1];
                    m->path_index += 2;
                }

                // If end of path, reset path to beginning
                if (m->next_px == (int8_t)0xea && m->next_py == (int8_t)0xea) {
                    m->next_px = game->level[game->cur_level].path[0];
                    m->next_py = game->level[game->cur_level].path[1];
                    m->path_index += 2;
                }

                if (m->next_px < 0) {
                    m->px -= 1;
                    m->next_px++;
                }
                if (m->next_px > 0) {
                    m->px += 1;
                    m->next_px--;
                }

                if (m->next_py < 0) {
                    m->py -= 1;
                    m->next_py++;
                }
                if (m->next_py > 0) {
                    m->py += 1;
                    m->next_py--;
                }
            }

            m->x = m->px / TILE_SIZE;
            m->y = m->py / TILE_SIZE;
        }
    }

    // enemies firing
    if (!game->ebullet_px && !game->ebullet_py) {
        for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
            if (game->enemies[i].type && is_visible(game->enemies[i].px) && !game->enemies[i].death_timer) {
                game->ebullet_dir = game->player.px < game->enemies[i].px ? -1 : 1;

                // Default direction of bullet should be right
                if (!game->ebullet_dir) {
                    game->ebullet_dir = 1;
                }

                // Create the bullet on the appropriate side of the enemy
                if (game->ebullet_dir == 1) {
                    game->ebullet_px = game->enemies[i].px + 18;
                }
                if (game->ebullet_dir == -1) {
                    game->ebullet_px = game->enemies[i].px - 8;
                }
                sprintf(debug_msgs[0], "bullet px: %d", game->ebullet_px);

                game->ebullet_py = game->enemies[i].py + 8;
            }
        }
    }
}

static void pickup_item(uint8_t grid_x, uint8_t grid_y)
{
    if (!grid_x || !grid_y) {
        return;
    }

    uint8_t type = game->level[game->cur_level].tiles[grid_y * 100 + grid_x];

    char pickup_msg[256];
    sprintf(pickup_msg, "picked up item: %d", type);
    LOG_INFO("pickup_item", pickup_msg, "something else");

    switch (type) {
    case TILE_JETPACK: {
        game->player.jetpack = 0xff;
    } break;

    case TILE_TROPHY: {
        add_score(SCORE_TROPHY);
        game->player.trophy = 1;
    } break;

    case TILE_GUN: {
        game->player.gun = 1;
    } break;

    // TODO:(lukefilewalker) pull these magic nums out
    case 47: {
        add_score(100);
    } break;

    case 48: {
        add_score(50);
    } break;

    case 49: {
        add_score(150);

    } break;
    case 50: {
        add_score(300);
    } break;

    case 51: {
        add_score(200);
    } break;

    case 52: {
        add_score(500);
    } break;

    default:
        break;
    }

    game->level[game->cur_level].tiles[grid_y * 100 + grid_x] = 0;

    game->player.check_pickup_x = 0;
    game->player.check_pickup_y = 0;
}

static void add_score(uint16_t new_score)
{
    if (game->player.score / SCORE_NEW_LIFE != game->player.score + new_score / SCORE_NEW_LIFE) {
        game->player.lives++;
    }
    game->player.score = new_score;
}

static void clear_input(void)
{
    game->player.try_right = 0;
    game->player.try_left = 0;
    game->player.try_jump = 0;
    game->player.try_down = 0;
    game->player.try_fire = 0;
    game->player.try_jetpack = 0;
}

static uint8_t update_frame(uint8_t tile, uint8_t salt)
{
    uint8_t mod;

    switch (tile) {
    case 6: {
        mod = 4;
    } break;

    case 10: {
        mod = 5;
    } break;

    case 25: {
        mod = 4;
    } break;

    case 36: {
        mod = 5;
    } break;

    case 129: {
        mod = 4;
    } break;

    default: {
        mod = 1;
    } break;
    }

    return tile + ((salt + game->tick) / 5) % mod;
}

static void render_world(void)
{
    uint8_t tile_index;
    SDL_Rect dest = {
        .w = TILE_SIZE,
        .h = TILE_SIZE,
    };

    for (int i = 0; i < 10; i++) {
        // Move everything down a tile for the UI
        dest.y = TILE_SIZE + i * TILE_SIZE;

        for (int j = 0; j < 20; j++) {
            dest.x = j * TILE_SIZE;

            tile_index = game->level[game->cur_level].tiles[i * 100 + game->view_x + j];
            tile_index = update_frame(tile_index, dest.x);
            SDL_RenderCopy(renderer, assets->gfx_tiles[tile_index], NULL, &dest);
        }
    }
}

static void render_player(void)
{
    SDL_Rect dest = {
        .x = game->player.px - game->view_x * TILE_SIZE,
        // Move player down a tile for the UI
        .y = TILE_SIZE + game->player.py,
        .w = PLAYER_W,
        .h = PLAYER_H,
    };

    uint8_t tile_index = TILE_PLAYER_STANDING;
    if (game->player.last_dir) {
        // TODO:(lukefilewalker) Check what these magic numbers are
        tile_index = game->player.last_dir > 0 ? 53 : 57;
        tile_index += (game->player.tick / 5) % 3;
    }

    if (game->player.using_jetpack) {
        tile_index = game->player.last_dir >= 0 ? TILE_JETPACK_LEFT : TILE_JETPACK_RIGHT;
    } else {
        if (game->player.jump || !game->player.on_ground) {
            tile_index = game->player.last_dir >= 0 ? TILE_PLAYER_JUMP_LEFT : TILE_PLAYER_JUMP_RIGHT;
        }

        if (game->player.climb) {
            tile_index = 71 + (game->player.tick / 5) % 3;
        }
    }

    if (game->player.death_timer) {
        // TODO:(lukefilewalker) for some reason macros freak the complier out here - find out why
        tile_index = 129 + ((game->player.tick / 3) % 4);
    }

    SDL_RenderCopy(renderer, assets->gfx_tiles[tile_index], NULL, &dest);
}

static void render_enemies(void)
{
    for (int i = 0; i < NUM_ENEMIES; i++) {
        enemy_t *m = &game->enemies[i];
        // TODO:(lukefilewalker) figure out whats going on with this magic num
        uint8_t tile_index = m->death_timer ? 129 : m->type;
        tile_index += (game->tick / 3) % 4;

        if (m->type) {
            SDL_Rect dest = {
                .x = m->px - game->view_x * TILE_SIZE,
                // Move player down a tile for the UI
                .y = TILE_SIZE + m->py,
                .w = PLAYER_W,
                .h = PLAYER_H,
            };

            SDL_RenderCopy(renderer, assets->gfx_tiles[tile_index], NULL, &dest);
        }
    }
}

static void render_player_bullet(void)
{
    if (game->player.bullet_px && game->player.bullet_py) {
        SDL_Rect dest = {
            .x = game->player.bullet_px - game->view_x * TILE_SIZE,
            // Move player down a tile for the UI
            .y = TILE_SIZE + game->player.bullet_py,
            .w = BULLET_W,
            .h = BULLET_H,
        };
        uint8_t tile_index = game->player.bullet_dir > 0 ? TILE_PLAYER_BULLET_LEFT : TILE_PLAYER_BULLET_RIGHT;
        SDL_RenderCopy(renderer, assets->gfx_tiles[tile_index], NULL, &dest);
    }
}

// TODO:(lukefilewalker): combine with enemy render?
static void render_enemies_bullet(void)
{
    if (game->ebullet_px && game->ebullet_py) {
        SDL_Rect dest = {
            .x = game->ebullet_px - game->view_x * TILE_SIZE,
            // Move player down a tile for the UI
            .y = TILE_SIZE + game->ebullet_py,
            .w = BULLET_W,
            .h = BULLET_H,
        };
        uint8_t tile_index = game->ebullet_dir > 0 ? TILE_ENEMY_BULLET_LEFT : TILE_ENEMY_BULLET_RIGHT;
        SDL_RenderCopy(renderer, assets->gfx_tiles[tile_index], NULL, &dest);
    }
}

// TODO:(lukefilewalker) pull out co-ords for items into some atlas or map or something
static void render_ui(void)
{
    // Draw UI frame
    SDL_Rect dest = {.x = 0, .y = 16, .w = 960, .h = 1};
    SDL_SetRenderDrawColor(renderer, COLOUR_WHITE, COLOUR_WHITE, COLOUR_WHITE, COLOUR_WHITE);
    SDL_RenderFillRect(renderer, &dest);
    dest.y = 176;
    SDL_RenderFillRect(renderer, &dest);

    // Score label
    dest.x = 1;
    dest.y = 2;
    dest.w = 62;
    dest.h = 11;
    SDL_RenderCopy(renderer, assets->gfx_tiles[TILE_UI_SCORE], NULL, &dest);

    // Level
    dest.x = 120;
    SDL_RenderCopy(renderer, assets->gfx_tiles[TILE_UI_LEVEL], NULL, &dest);

    // Lives
    dest.x = 200;
    SDL_RenderCopy(renderer, assets->gfx_tiles[TILE_UI_LIVES], NULL, &dest);

    // Player score
    dest.x = 64;
    dest.w = 8;
    dest.h = 11;
    SDL_RenderCopy(renderer, assets->gfx_tiles[TILE_UI_NUM_0 + (game->player.score / 10000) % 10], NULL, &dest);
    dest.x = 72;
    SDL_RenderCopy(renderer, assets->gfx_tiles[TILE_UI_NUM_0 + (game->player.score / 1000) % 10], NULL, &dest);
    dest.x = 80;
    SDL_RenderCopy(renderer, assets->gfx_tiles[TILE_UI_NUM_0 + (game->player.score / 100) % 10], NULL, &dest);
    dest.x = 88;
    SDL_RenderCopy(renderer, assets->gfx_tiles[TILE_UI_NUM_0 + (game->player.score / 10) % 10], NULL, &dest);
    dest.x = 96;
    SDL_RenderCopy(renderer, assets->gfx_tiles[TILE_UI_NUM_0 + (game->player.score) % 10], NULL, &dest);

    // Current level
    dest.x = 170;
    SDL_RenderCopy(renderer, assets->gfx_tiles[TILE_UI_NUM_0 + (game->cur_level + 1) / 10], NULL, &dest);
    dest.x = 178;
    SDL_RenderCopy(renderer, assets->gfx_tiles[TILE_UI_NUM_0 + (game->cur_level + 1) % 10], NULL, &dest);

    // Player lives
    for (int i = 0; i < game->player.lives; i++) {
        dest.x = (255 + 16 * i);
        dest.w = 16;
        dest.h = 12;
        SDL_RenderCopy(renderer, assets->gfx_tiles[TILE_UI_LIFE], NULL, &dest);
    }

    // Trophy icon
    if (game->player.trophy) {
        dest.x = 72;
        dest.y = 180;
        dest.w = 176;
        dest.h = 14;
        SDL_RenderCopy(renderer, assets->gfx_tiles[138], NULL, &dest);
    }

    // Gun icon
    if (game->player.gun) {
        dest.x = 255;
        dest.y = 180;
        dest.w = 62;
        dest.h = 11;
        SDL_RenderCopy(renderer, assets->gfx_tiles[134], NULL, &dest);
    }

    // Jetpack
    if (game->player.jetpack) {
        dest.x = 1;
        dest.y = 177;
        dest.w = 62;
        dest.h = 11;
        SDL_RenderCopy(renderer, assets->gfx_tiles[133], NULL, &dest);

        dest.x = 1;
        dest.y = 190;
        dest.h = 8;
        SDL_RenderCopy(renderer, assets->gfx_tiles[141], NULL, &dest);

        dest.x = 2;
        dest.y = 192;
        dest.w = game->player.jetpack * 0.23;
        dest.h = 4;
        SDL_SetRenderDrawColor(renderer, 0xee, COLOUR_BLACK, COLOUR_BLACK, COLOUR_WHITE);
        SDL_RenderFillRect(renderer, &dest);
    }
}

static void render_debug_ui(void)
{
    if (strlen(debug_msgs[0]) == 0) {
        return;
    }

    SDL_Color text_colour = {255, 255, 255, 255};
    SDL_Surface *line_surfaces[MAX_DEBUG_MESSAGES] = {0};
    uint16_t tot_height = 0, longest_line = 0;

    // Create each line's surface and calculate the total height of al the lines
    for (size_t i = 0; i < num_debug_msgs; i++) {
        if (strlen(debug_msgs[i]) == 0) {
            break;
        }

        line_surfaces[i] = TTF_RenderText_Solid(font, debug_msgs[i], text_colour);
        if (!line_surfaces[i]) {
            SDL_Log("Unable to create debug line surface! TTF_Error: %s", TTF_GetError());
            return;
        }

        tot_height += line_surfaces[i]->h;
        if (line_surfaces[i]->w > longest_line) {
            longest_line = line_surfaces[i]->w;
        }
    }

    // Create the final destination surface
    SDL_Surface *final_surface =
        SDL_CreateRGBSurfaceWithFormat(0, longest_line, tot_height, 32, SDL_PIXELFORMAT_RGBA32);
    if (!final_surface) {
        SDL_Log("Unable to create final debug surface! SDL_Error: %s", SDL_GetError());
        return;
    }

    SDL_FillRect(final_surface, NULL, SDL_MapRGBA(final_surface->format, 0, 0, 0, (uint8_t)(255 * 0.75)));

    // Copy each line surface to the destination
    for (size_t i = 0; i < num_debug_msgs; i++) {
        SDL_Rect destRect = {0, line_surfaces[i]->h * i, line_surfaces[i]->w, line_surfaces[i]->h};
        SDL_BlitSurface(line_surfaces[i], NULL, final_surface, &destRect);
        SDL_FreeSurface(line_surfaces[i]);
    }

    // Create final texture
    SDL_Texture *debug_texture = SDL_CreateTextureFromSurface(renderer, final_surface);
    if (!debug_texture) {
        SDL_Log("Unable to create texture from surface! SDL_Error: %s", SDL_GetError());
        return;
    }

    SDL_Rect renderQuad = {5, 5, final_surface->w, tot_height};
    SDL_FreeSurface(final_surface);

    // Draw background
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    // SDL_RenderFillRect(renderer, &renderQuad);

    SDL_RenderCopy(renderer, debug_texture, NULL, &renderQuad);

    SDL_DestroyTexture(debug_texture);
}

static uint8_t is_clear(uint16_t px, uint16_t py, uint8_t is_player)
{
    uint8_t grid_x = px / TILE_SIZE;
    uint8_t grid_y = py / TILE_SIZE;

    if (grid_x >= GAME_AREA_TOP || grid_y >= GAME_AREA_BOTTOM) {
        return 1;
    }

    uint8_t type = game->level[game->cur_level].tiles[grid_y * 100 + grid_x];

    // Tiles that the player collides with
    switch (type) {
    case 1:
    case 3:
    case 5:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 21:
    case 22:
    case 23:
    case 24:
    case 29:
    case 30: {
        return 0;
    } break;

    default:
        break;
    }

    if (is_player) {
        switch (type) {
        case TILE_DOOR: {
            game->player.check_door = 1;
        } break;

        case TILE_GUN: {
            game->player.gun = 1;
#ifdef _MSC_VER
            __fallthrough;
#else
            __attribute__((fallthrough));
#endif
        };
        case TILE_JETPACK:
        case TILE_TROPHY:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
        case 52: {
            game->player.check_pickup_x = grid_x;
            game->player.check_pickup_y = grid_y;
        } break;

        case 6:
        case 25:
        case 36: {
            if (!game->player.death_timer) {
                game->player.death_timer = DEATH_TIME;
            }
        } break;

        default:
            break;
        }
    }

    return 1;
}

static inline uint8_t is_visible(uint16_t px)
{
    uint8_t posx = px / TILE_SIZE;
    return posx - game->view_x < 20 && posx - game->view_x >= 0;
}

static void add_debug_msg(char *format, char *msg)
{
    // TODO:(lukefilewalker): create a circular buffer for the messages
    if (num_debug_msgs > MAX_DEBUG_MESSAGES) {
        LOG_INFO("debug messages", "we've run out of space :(");
        return;
    }
    sprintf(debug_msgs[num_debug_msgs++], format, msg);
}

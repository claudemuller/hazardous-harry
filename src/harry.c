#include "harry.h"
#include "error.h"
#include "log.h"
#include <SDL2/SDL_ttf.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static game_state_t *game = {0};
static game_assets_t *assets = {0};
static SDL_Window *window;
static SDL_Renderer *renderer;
static TTF_Font *font;

// TODO:(lukefilewalker): make this better :(
#define MAX_DEBUG_MESSAGES 20
static uint8_t num_debug_msgs = 0;
static char debug_msgs[MAX_DEBUG_MESSAGES][1000] = {0};

static int init_assets(void);

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
static void move_monsters(float dt);
static void pickup_item(uint8_t, uint8_t);
static void clear_input(void);

static void render(void);
static void render_world(void);
static void render_player(void);
static void render_monsters(void);
// TODO:(lukefilewalker) combine these into render funcs?
static void render_player_bullet(void);
static void render_monsters_bullet(void);
static void render_debug_ui(void);

static uint8_t is_clear(uint16_t px, uint16_t py, uint8_t is_player);
static uint8_t is_visible(uint16_t px);
static void add_debug_msg(char *format, char *msg);

int game_init(const bool debug)
{
    LOG_INFO("game_init", "initialising game");

    FILE *fd_level;
    char fname[ASSET_FNAME_SIZE];
    char file_num[4];

    game = malloc(sizeof(game_state_t));
    if (!game) {
        return err_fatal(ERR_ALLOC, "game state");
    }

    game->debug = debug;
    game->is_running = false;
    game->ticks_last_frame = SDL_GetTicks();
    game->cur_level = 2;
    // TODO:(lukefilewalker): remove this init code when you've confirmed that game data is init'd to 0
    game->scroll_x = 0;

    game->player.on_ground = 1;
    game->player.lives = NUM_START_LIVES;
    // TODO:(lukefilewalker): remove this init code when you've confirmed that player is init'd to 0
    game->player.try_right = 0;
    game->player.try_left = 0;
    game->player.try_jump = 0;
    game->player.try_jetpack = 0;
    game->player.check_pickup_x = 0;
    game->player.check_pickup_y = 0;

    for (int i = 0; i < NUM_MONSTERS; i++) {
        game->monsters[i].type = 0;
    }

    log_info("game_init", "loading levels");

    for (int i = 0; i < NUM_LEVELS; i++) {
        fname[0] = '\0';
        strcat(fname, "res/level");
        sprintf(&file_num[0], "%u", i);
        strcat(fname, file_num);
        strcat(fname, ".dat");

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

    log_info("game_init", "initialising SDL");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return err_fatal(ERR_SDL_INIT, SDL_GetError());
    }

    // TODO:(lukefilewalker): clean up :(
    if (TTF_Init() == -1) {
        SDL_Log("SDL_ttf could not initialize! TTF_Error: %s", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    if (SDL_CreateWindowAndRenderer(320 * DISPLAY_SCALE, 200 * DISPLAY_SCALE, 0, &window, &renderer) != 0) {
        return err_fatal(ERR_SDL_CREATE_WIN_RENDER, SDL_GetError());
    }

    SDL_RenderSetScale(renderer, DISPLAY_SCALE, DISPLAY_SCALE);

    // TODO:(lukefilewalker): clean up :(
    font = TTF_OpenFont("/home/lukefilewalker/repos/tyler.c/assets/fonts/DroidSans.ttf", 16);
    if (!font) {
        SDL_Log("Font could not be loaded! TTF_Error: %s", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    log_info("game_init", "malloc'ing assets");

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
    log_info("game_run", "running game");

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
    log_info("game_destroy", "cleaning up");

    free(assets);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(game);

    return SUCCESS;
}

static int init_assets(void)
{
    log_info("init_assets", "entered");

    char fname[ASSET_FNAME_SIZE];
    char file_num[4];
    char mname[ASSET_FNAME_SIZE];
    char mask_num[4];
    SDL_Surface *surface;
    SDL_Surface *mask_surface;
    uint8_t mask_offset;
    uint8_t *player_pixels;
    uint8_t *mask_pixels;

    for (size_t i = 0; i < NUM_TILES; i++) {
        fname[0] = '\0';
        strcat(fname, "res/tile");
        sprintf(&file_num[0], "%u", (int)i);
        strcat(fname, file_num);
        strcat(fname, ".bmp");

        // Harry tiles
        if ((i >= 53 && i <= 59) || i == 67 || i == 68 || (i >= 71 && i <= 73) || (i >= 77 && i <= 82)) {
            if (i >= 53 && i <= 59)
                mask_offset = 7;
            if (i >= 67 && i <= 68)
                mask_offset = 2;
            if (i >= 71 && i <= 73)
                mask_offset = 3;
            if (i >= 77 && i <= 82)
                mask_offset = 6;

            surface = SDL_LoadBMP(fname);
            if (!surface) {
                return err_fatal(ERR_SDL_LOADING_BMP, fname);
            }
            player_pixels = (uint8_t *)surface->pixels;

            mname[0] = '\0';
            strcat(mname, "res/tile");
            sprintf(&mask_num[0], "%u", (uint8_t)i + mask_offset);
            strcat(mname, mask_num);
            strcat(mname, ".bmp");

            mask_surface = SDL_LoadBMP(mname);
            if (!mask_surface) {
                return err_fatal(ERR_SDL_LOADING_BMP, mname);
            }
            mask_pixels = (uint8_t *)mask_surface->pixels;

            // Go through tile and make pixels white where they aren't black
            for (int j = 0; j < mask_surface->pitch * mask_surface->h; j++) {
                player_pixels[j] = mask_pixels[j] ? COLOUR_WHITE : player_pixels[j];
            }
            SDL_SetColorKey(surface, 1, SDL_MapRGB(surface->format, COLOUR_WHITE, COLOUR_WHITE, COLOUR_WHITE));
            assets->gfx_tiles[i] = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_FreeSurface(surface);
            SDL_FreeSurface(mask_surface);
        } else {
            surface = SDL_LoadBMP(fname);
            if (!surface) {
                return err_fatal(ERR_SDL_LOADING_BMP, fname);
            }

            assets->gfx_tiles[i] = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_FreeSurface(surface);
        }
    }

    return SUCCESS;
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
    game->player.on_ground = !game->player.collision_point[4] && !game->player.collision_point[5];
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
    move_monsters(dt);
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
    render_monsters();
    render_player_bullet();
    render_monsters_bullet();

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
    if (game->player.jetpack_delay) {
        game->player.jetpack_delay--;
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

    for (size_t i = 0; i < NUM_MONSTERS; i++) {
        if (game->monsters[i].death_timer) {
            game->monsters[i].death_timer--;
            // Monster has died
            if (!game->monsters[i].death_timer) {
                game->monsters[i].type = 0;
            }
        } else {
            if (game->monsters[i].type) {
                // If player and monster collide, everyone dies
                if (game->monsters[i].x == game->player.x && game->monsters[i].y == game->player.y) {
                    game->player.death_timer = DEATH_TIME;
                    game->monsters[i].death_timer = DEATH_TIME;
                }
            }
        }
    }
}

static void start_level(void)
{
    add_debug_msg("level: %s", "1");

    restart_level();

    for (int i = 0; i < NUM_MONSTERS; i++) {
        game->monsters[i].type = 0;
    }

    switch (game->cur_level) {
    case 2: {
        game->monsters[0].type = TILE_MONSTER_SPIDER;
        game->monsters[0].path_index = 0;
        game->monsters[0].px = 44 * TILE_SIZE;
        game->monsters[0].py = 4 * TILE_SIZE;
        game->monsters[0].next_px = 0;
        game->monsters[0].next_py = 0;
        game->monsters[0].death_timer = 0;

        game->monsters[1].type = TILE_MONSTER_SPIDER;
        game->monsters[1].path_index = 0;
        game->monsters[1].px = 59 * TILE_SIZE;
        game->monsters[1].py = 4 * TILE_SIZE;
        game->monsters[1].next_px = 0;
        game->monsters[1].next_py = 0;
        game->monsters[1].death_timer = 0;
    } break;

    case 3: {
        game->monsters[0].type = TILE_MONSTER_PURPER;
        game->monsters[0].path_index = 0;
        game->monsters[0].px = 32 * TILE_SIZE;
        game->monsters[0].py = 2 * TILE_SIZE;
        game->monsters[0].next_px = 0;
        game->monsters[0].next_py = 0;
        game->monsters[0].death_timer = 0;
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

        for (size_t i = 0; i < NUM_MONSTERS; i++) {
            if (game->monsters[i].type) {
                uint8_t mx = game->monsters[i].x;
                uint8_t my = game->monsters[i].y;

                if ((grid_y == my || grid_y == my + 1) && (grid_x == mx || grid_x == mx + 1)) {
                    game->player.bullet_px = game->player.bullet_py = 0;
                    game->monsters[i].death_timer = DEATH_TIME;
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
        game->player.collision_point[0] && game->player.collision_point[1]) {
        game->player.jump = 1;
    }

    if (game->player.try_fire && game->player.gun && !game->player.bullet_px && !game->player.bullet_py) {
        game->player.fire = 1;
    }

    if (game->player.try_jetpack && game->player.jetpack && !game->player.jetpack_delay) {
        game->player.using_jetpack = !game->player.using_jetpack;
        game->player.jetpack_delay = 10;
    }

    if (game->player.try_down && game->player.using_jetpack && game->player.collision_point[4] &&
        game->player.collision_point[5]) {
        game->player.down = 1;
    }

    if (game->player.try_jump && game->player.using_jetpack && game->player.collision_point[0] &&
        game->player.collision_point[1]) {
        game->player.up = 1;
    }
}

static void move_player(float dt)
{
    game->player.x = game->player.px / TILE_SIZE;
    game->player.y = game->player.py / TILE_SIZE;

    if (game->player.right) {
        // float px = PLAYER_MOVE; // * MUL * dt;
        game->player.px += PLAYER_MOVE;
        game->player.right = 0;
        game->player.last_dir = 1;
    }
    if (game->player.left) {
        // float px = PLAYER_MOVE; // * MUL * dt;
        game->player.px -= PLAYER_MOVE;
        game->player.left = 0;
        game->player.last_dir = -1;
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
            game->player.jump_timer = 45;
            game->player.last_dir = 0;
        }

        // TODO:(lukefilewalker): add delta time to jump
        if (game->player.collision_point[0] && game->player.collision_point[1]) {
            if (game->player.jump_timer > 10) {
                game->player.py -= PLAYER_MOVE;
            }
            if (game->player.jump_timer >= 5 && game->player.jump_timer <= 10) {
                game->player.py -= PLAYER_MOVE / 2;
            }
        }

        game->player.jump_timer--;

        if (game->player.jump_timer == 0) {
            game->player.jump = 0;
        }
    }

    // Add gravity
    if (!game->player.jump && !game->player.on_ground && !game->player.using_jetpack) {
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

static void move_monsters(float dt)
{
    for (uint8_t i = 0; i < NUM_MONSTERS; i++) {
        monster_t *m = &game->monsters[i];
        if (m->type && !m->death_timer) {
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

            m->x = m->px / TILE_SIZE;
            m->y = m->py / TILE_SIZE;
        }
    }

    // Monsters firing
    if (!game->ebullet_px && !game->ebullet_py) {
        for (uint8_t i = 0; i < NUM_MONSTERS; i++) {
            if (game->monsters[i].type && is_visible(game->monsters[i].px) && !game->monsters[i].death_timer) {
                game->ebullet_dir = game->player.px < game->monsters[i].px ? -1 : 1;

                // Default direction of bullet should be right
                if (!game->ebullet_dir) {
                    game->ebullet_dir = 1;
                }

                // Create the bullet on the appropriate side of the monster
                if (game->ebullet_dir == 1) {
                    game->ebullet_px = game->monsters[i].px + 18;
                }
                if (game->ebullet_dir == -1) {
                    game->ebullet_px = game->monsters[i].px - 8;
                }
                sprintf(debug_msgs[0], "bullet px: %d", game->ebullet_px);

                game->ebullet_py = game->monsters[i].py + 8;
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
        game->player.score += SCORE_TROPHY;
        game->player.trophy = 1;
    } break;

    case TILE_GUN: {
        game->player.gun = 1;
    } break;

    default:
        break;
    }

    game->level[game->cur_level].tiles[grid_y * 100 + grid_x] = 0;

    game->player.check_pickup_x = 0;
    game->player.check_pickup_y = 0;
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

static void render_world(void)
{
    uint8_t tile_index;
    SDL_Rect dest = {
        .w = TILE_SIZE,
        .h = TILE_SIZE,
    };

    for (int i = 0; i < 10; i++) {
        dest.y = i * TILE_SIZE;

        for (int j = 0; j < 20; j++) {
            dest.x = j * TILE_SIZE;

            tile_index = game->level[game->cur_level].tiles[i * 100 + game->view_x + j];
            SDL_RenderCopy(renderer, assets->gfx_tiles[tile_index], NULL, &dest);
        }
    }
}

static void render_player(void)
{
    SDL_Rect dest = {
        .x = game->player.px - game->view_x * TILE_SIZE,
        .y = game->player.py,
        .w = PLAYER_W,
        .h = PLAYER_H,
    };

    uint8_t tile_index = TILE_PLAYER_STANDING;

    if (game->player.using_jetpack) {
        tile_index = game->player.last_dir >= 0 ? TILE_JETPACK_LEFT : TILE_JETPACK_RIGHT;
    } else {
        if (game->player.jump || !game->player.on_ground) {
            tile_index = game->player.last_dir >= 0 ? TILE_PLAYER_JUMP_LEFT : TILE_PLAYER_JUMP_RIGHT;
        }
    }

    if (game->player.death_timer) {
        tile_index = TILE_DEATH;
    }

    SDL_RenderCopy(renderer, assets->gfx_tiles[tile_index], NULL, &dest);
}

static void render_monsters(void)
{
    for (int i = 0; i < NUM_MONSTERS; i++) {
        monster_t *m = &game->monsters[i];
        // TODO:(lukefilewalker) figure out whats going on with this magic num
        uint8_t tile_index = m->death_timer ? 129 : m->type;

        if (m->type) {
            SDL_Rect dest = {
                .x = m->px - game->view_x * TILE_SIZE,
                .y = m->py,
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
            .y = game->player.bullet_py,
            .w = BULLET_W,
            .h = BULLET_H,
        };
        uint8_t tile_index = game->player.bullet_dir > 0 ? TILE_PLAYER_BULLET_LEFT : TILE_PLAYER_BULLET_RIGHT;
        SDL_RenderCopy(renderer, assets->gfx_tiles[tile_index], NULL, &dest);
    }
}

// TODO:(lukefilewalker): combine with monster render?
static void render_monsters_bullet(void)
{
    if (game->ebullet_px && game->ebullet_py) {
        SDL_Rect dest = {
            .x = game->ebullet_px - game->view_x * TILE_SIZE,
            .y = game->ebullet_py,
            .w = BULLET_W,
            .h = BULLET_H,
        };
        uint8_t tile_index = game->ebullet_dir > 0 ? TILE_MONSTER_BULLET_LEFT : TILE_MONSTER_BULLET_RIGHT;
        SDL_RenderCopy(renderer, assets->gfx_tiles[tile_index], NULL, &dest);
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

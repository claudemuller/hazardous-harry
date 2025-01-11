#include "harry.h"
#include "error.h"
#include "log.h"
#include <SDL2/SDL_ttf.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static game_state_t *game;
static game_assets_t *assets;
SDL_Window *window;
SDL_Renderer *renderer;
TTF_Font *font;
char debug_text[1000] = {0};

static int init_assets(void);

static void check_collisions(void);
static void process_input(void);
static void update(float dt);
static void scroll_screen(void);
static void update_level(void);
static void start_level(void);
static void update_pbullet(void);
static void verify_input(void);
static void move_player(float dt);
static void pickup_item(uint8_t, uint8_t);
static void clear_input(void);

static void render(void);
static void render_world(void);
static void render_player(void);
static void render_bullet(void);
static void render_debug_ui(void);

static uint8_t is_clear(uint16_t px, uint16_t py, uint8_t is_player);

int game_init(void)
{
    log_info("game_init", "initialising game");

    FILE *fd_level;
    char fname[ASSET_FNAME_SIZE];
    char file_num[4];

    game = malloc(sizeof(game_state_t));
    if (!game) {
        return err_fatal(ERR_ALLOC, "game state");
    }

    game->is_running = false;
    game->ticks_last_frame = SDL_GetTicks();
    game->cur_level = 2;
    game->scroll_x = 0;

    game->player.on_ground = 1;
    game->player.try_right = 0;
    game->player.try_left = 0;
    game->player.try_jump = 0;
    game->player.try_jetpack = 0;
    game->player.check_pickup_x = 0;
    game->player.check_pickup_y = 0;

    log_info("game_init", "loading levels");

    for (int i = 0; i < 10; i++) {
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

    // TODO(claude): clean up :(
    if (TTF_Init() == -1) {
        SDL_Log("SDL_ttf could not initialize! TTF_Error: %s", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    if (SDL_CreateWindowAndRenderer(320 * DISPLAY_SCALE, 200 * DISPLAY_SCALE, 0, &window, &renderer) != 0) {
        return err_fatal(ERR_SDL_CREATE_WIN_RENDER, SDL_GetError());
    }

    SDL_RenderSetScale(renderer, DISPLAY_SCALE, DISPLAY_SCALE);

    // TODO(claude): clean up :(
    font = TTF_OpenFont("/home/lukefilewalker/repos/tyler.c/assets/fonts/DroidSans.ttf", 10);
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
    // TODO(claude): change to is_colliding
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
    verify_input();
    move_player(dt);
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
    render_bullet();
    render_debug_ui();

    SDL_RenderPresent(renderer);
}

static void scroll_screen(void)
{
    sprintf(debug_text, "player.x = %d\n", game->player.x);
    sprintf(debug_text, "game->view_x = %d\n", game->view_x);
    sprintf(debug_text, "player.x - game->view_x = %d", game->player.x - game->view_x);

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
}

static void start_level(void)
{
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
    }

    game->player.px = game->player.x * TILE_SIZE;
    game->player.py = game->player.y * TILE_SIZE;
    game->player.trophy = 0;
    game->player.gun = 0;
    game->player.fire = 0;
    game->player.using_jetpack = 0;
    game->player.check_door = 0;
    game->player.jump_timer = 0;
    game->view_x = 0;
    game->view_y = 0;
    game->player.last_dir = 0;
    game->player.pbullet_px = 0;
    game->player.pbullet_py = 0;
    game->player.pbullet_dir = 0;
    game->player.ebullet_px = 0;
    game->player.ebullet_py = 0;
    game->player.ebullet_dir = 0;
}

static void update_pbullet(void)
{
    uint8_t grid_x;

    if (!game->player.pbullet_px || !game->player.pbullet_py) {
        return;
    }

    game->player.pbullet_px += game->player.pbullet_dir * BULLET_SPEED;

    // If bullet hits a collidable tile, remove it
    if (!is_clear(game->player.pbullet_px, game->player.pbullet_py, 0)) {
        game->player.pbullet_px = game->player.pbullet_py = 0;
    }

    grid_x = game->player.pbullet_px / TILE_SIZE;

    // If bullet reaches the end of the screen, remove it
    if (grid_x - game->view_x < 1 || grid_x - game->view_x > 20) {
        game->player.pbullet_px = game->player.pbullet_py = 0;
    }
}

static void verify_input(void)
{
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

    if (game->player.try_fire && game->player.gun && !game->player.pbullet_px && !game->player.pbullet_py) {
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
    // const float MUL = 45.0f;

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
            game->player.jump_timer = 55;
        }

        // TODO(claude): add delta time to jump
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
        game->player.pbullet_dir = game->player.last_dir;

        if (!game->player.pbullet_dir) {
            game->player.pbullet_dir = 1;
        }

        if (game->player.pbullet_dir == 1) {
            game->player.pbullet_px = game->player.px + 18;
        }

        if (game->player.pbullet_dir == -1) {
            game->player.pbullet_px = game->player.px - 8;
        }

        game->player.pbullet_py = game->player.py + 8;
        game->player.fire = 0;
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
    uint8_t tile_index = TILE_PLAYER_STANDING;
    SDL_Rect dest = {
        .x = game->player.px - game->view_x * TILE_SIZE,
        .y = game->player.py,
        .w = PLAYER_W,
        .h = PLAYER_H,
    };

    // TODO(claude): fix the messed up jetpack :/
    if (game->player.using_jetpack) {
        tile_index = game->player.last_dir >= 0 ? TILE_JETPACK_LEFT : TILE_JETPACK_RIGHT;
    } else {
        if (game->player.jump || !game->player.on_ground) {
            tile_index = game->player.last_dir >= 0 ? TILE_PLAYER_JUMP_LEFT : TILE_PLAYER_JUMP_RIGHT;
        }
    }

    SDL_RenderCopy(renderer, assets->gfx_tiles[tile_index], NULL, &dest);
}

static void render_bullet(void)
{
    if (game->player.pbullet_px && game->player.pbullet_py) {
        SDL_Rect dest = {
            .x = game->player.pbullet_px - game->view_x * TILE_SIZE,
            .y = game->player.pbullet_py,
            .w = BULLET_W,
            .h = BULLET_H,
        };
        uint8_t tile_index = game->player.pbullet_dir > 0 ? TILE_BULLET_LEFT : TILE_BULLET_RIGHT;
        SDL_RenderCopy(renderer, assets->gfx_tiles[tile_index], NULL, &dest);
    }
}

static void render_debug_ui(void)
{
    SDL_Color textColor = {255, 255, 255, 255}; // White color
    SDL_Surface *surface = TTF_RenderText_Solid(font, debug_text, textColor);
    if (!surface) {
        SDL_Log("Unable to create text surface! TTF_Error: %s", TTF_GetError());
        return;
    }

    // Create texture from surface
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!textTexture) {
        SDL_Log("Unable to create texture from surface! SDL_Error: %s", SDL_GetError());
        return;
    }

    SDL_FreeSurface(surface);

    SDL_Rect renderQuad = {5, 5, surface->w, surface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
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

        case TILE_JETPACK: {
            game->player.using_jetpack = 1;
        };
        case TILE_GUN: {
            game->player.gun = 1;
        };
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

        default:
            break;
        }
    }

    return 1;
}

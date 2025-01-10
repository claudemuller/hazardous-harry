#include "harry.h"
#include "error.h"
#include "log.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static game_state_t *game;
static game_assets_t *assets;
SDL_Window *window;
SDL_Renderer *renderer;

static int init_assets(void);

static void check_collisions(void);
static void process_input(void);
static void update(void);
static void scroll_screen(void);
static void check_player_move(void);
static void move_player(void);
static void clear_input(void);

static void render(void);
static void render_world(void);
static void render_player(void);

static uint8_t is_clear(uint16_t px, uint16_t py);

int game_init(void)
{
    log_info("game_init", "entered");

    FILE *fd_level;
    char fname[ASSET_FNAME_SIZE];
    char file_num[4];

    game = malloc(sizeof(game_state_t));
    if (!game) {
        return err_fatal(ERR_ALLOC, "game state");
    }

    game->is_running = false;
    game->view_x = 0;
    game->view_y = 0;
    game->cur_level = 0;
    game->scroll_x = 0;

    game->player.x = PLAYER_START_X;
    game->player.y = PLAYER_START_Y;
    game->player.px = game->player.x * TILE_SIZE;
    game->player.py = game->player.y * TILE_SIZE;
    game->player.jump_timer = 0;
    game->player.on_ground = 1;

    log_info("game_init", "load levels");

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

    log_info("game_init", "init sdl");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return err_fatal(ERR_SDL_INIT, SDL_GetError());
    }

    if (SDL_CreateWindowAndRenderer(320 * DISPLAY_SCALE, 200 * DISPLAY_SCALE, 0, &window, &renderer) != 0) {
        return err_fatal(ERR_SDL_CREATE_WIN_RENDER, SDL_GetError());
    }

    SDL_RenderSetScale(renderer, DISPLAY_SCALE, DISPLAY_SCALE);

    log_info("game_init", "malloc assets");

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
    log_info("game_run", "entered");

    uint32_t timer_start, timer_end, delay;

    while (game->is_running) {
        timer_start = SDL_GetTicks();

        check_collisions();
        process_input();
        update();
        render();

        timer_end = SDL_GetTicks();

        delay = FPS - (timer_end - timer_start);
        delay = delay > 33 ? 0 : delay;

        // char fps_msg[256];
        // sprintf(fps_msg, "timer_start: %d - timer_end: %d", timer_start, timer_end);
        // log_info("game_run", fps_msg);

        SDL_Delay(delay);
    }

    return SUCCESS;
}

int game_destroy(void)
{
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

    for (size_t i = 0; i < NUM_TILES; i++) {
        fname[0] = '\0';
        strcat(fname, "res/tile");
        sprintf(&file_num[0], "%u", (int)i);
        strcat(fname, file_num);
        strcat(fname, ".bmp");

        SDL_Surface *surface = SDL_LoadBMP(fname);
        if (!surface) {
            return err_fatal(ERR_SDL_LOADING_BMP, fname);
        }

        assets->gfx_tiles[i] = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_FreeSurface(surface);
    }

    return SUCCESS;
}

static void check_collisions(void)
{
    // TODO(claude): change to is_colliding
    game->player.collision_point[0] = is_clear(game->player.px + 4, game->player.py - 1);
    game->player.collision_point[1] = is_clear(game->player.px + 10, game->player.py - 1);
    game->player.collision_point[2] = is_clear(game->player.px + 11, game->player.py + 4);
    game->player.collision_point[3] = is_clear(game->player.px + 11, game->player.py + 12);
    game->player.collision_point[4] = is_clear(game->player.px + 10, game->player.py + 16);
    game->player.collision_point[5] = is_clear(game->player.px + 4, game->player.py + 16);
    game->player.collision_point[6] = is_clear(game->player.px + 3, game->player.py + 12);
    game->player.collision_point[7] = is_clear(game->player.px + 3, game->player.py + 4);
    game->player.on_ground = !game->player.collision_point[4] && !game->player.collision_point[5];
}

static void process_input(void)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    // const uint8_t *keystate = SDL_GetKeyboardState(NULL);
    // if (keystate[SDL_SCANCODE_RIGHT]) {
    //     game->player.try_right = 1;
    // }
    // if (keystate[SDL_SCANCODE_LEFT]) {
    //     game->player.try_left = 1;
    // }
    // if (keystate[SDL_SCANCODE_UP]) {
    //     game->player.try_jump = 1;
    // }
    //
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

static void update(void)
{
    check_player_move();
    move_player();
    scroll_screen();
    clear_input();
}

static void render(void)
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    render_world();
    render_player();

    SDL_RenderPresent(renderer);
}

static void scroll_screen(void)
{
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

static void check_player_move(void)
{
    if (game->player.try_right && game->player.collision_point[2] && game->player.collision_point[3]) {
        game->player.right = game->player.try_right;
    }
    if (game->player.try_left && game->player.collision_point[6] && game->player.collision_point[7]) {
        game->player.left = game->player.try_left;
    }
    if (game->player.try_jump && game->player.on_ground && !game->player.jump && game->player.collision_point[0] &&
        game->player.collision_point[1]) {
        game->player.jump = game->player.try_jump;
    }
}

static void move_player(void)
{
    if (game->player.right) {
        game->player.px += PLAYER_MOVE;
        game->player.right = 0;
    }
    if (game->player.left) {
        game->player.px -= PLAYER_MOVE;
        game->player.left = 0;
    }
    if (game->player.jump) {
        if (!game->player.jump_timer) {
            game->player.jump_timer = 20;
        }

        if (game->player.collision_point[0] && game->player.collision_point[1]) {
            if (game->player.jump_timer > 5) {
                game->player.py -= 2;
            } else {
                game->player.py -= 1;
            }

            game->player.jump_timer--;
        } else {
            game->player.jump_timer = 0;
        }

        if (game->player.jump_timer == 0) {
            game->player.jump = 0;
        }
    }
}

static void clear_input(void)
{
    game->player.try_right = 0;
    game->player.try_left = 0;
    game->player.try_jump = 0;
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
        .x = game->player.px,
        .y = game->player.py,
        .w = PLAYER_W,
        .h = PLAYER_H,
    };
    SDL_RenderCopy(renderer, assets->gfx_tiles[PLAYER_TILE], NULL, &dest);
}

static uint8_t is_clear(uint16_t px, uint16_t py)
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

    default: {
        return 1;
    } break;
    }
}

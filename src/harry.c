#include "harry.h"
#include "SDL.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "SDL_timer.h"
#include "error.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static struct game_state_t *game;
static struct game_assets_t *assets;
SDL_Window *window;
SDL_Renderer *renderer;

int game_init(void)
{
    FILE *fd_level;
    char fname[ASSET_FNAME_SIZE];
    char file_num[4];

    game = malloc(sizeof(struct game_state_t));
    if (!game) {
        return err_fatal(ERR_ALLOC, "game state");
    }

    game->is_running = false;
    game->view_x = 0;
    game->view_y = 0;
    game->cur_level = 0;
    game->scroll_x = 0;
    game->player_x = PLAYER_START_X;
    game->player_y = PLAYER_START_Y;
    game->player_px = game->player_x * TILE_SIZE;
    game->player_py = game->player_y * TILE_SIZE;

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

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return err_fatal(ERR_SDL_INIT, SDL_GetError());
    }

    if (SDL_CreateWindowAndRenderer(320 * DISPLAY_SCALE, 200 * DISPLAY_SCALE, 0, &window, &renderer) != 0) {
        return err_fatal(ERR_SDL_CREATE_WIN_RENDER, SDL_GetError());
    }

    SDL_RenderSetScale(renderer, DISPLAY_SCALE, DISPLAY_SCALE);

    assets = malloc(sizeof(struct game_assets_t));
    if (!assets) {
        return err_fatal(ERR_ALLOC, "game assets");
    }
    int err = game_init_assets();
    if (err != SUCCESS) {
        return err_fatal(err, NULL);
    }

    game->is_running = true;

    return SUCCESS;
}

int game_run(void)
{
    uint32_t timer_start, timer_end, delay;

    while (game->is_running) {
        timer_start = SDL_GetTicks();

        game_process_input();
        game_update();
        game_render();

        timer_end = SDL_GetTicks();

        delay = FPS - (timer_end - timer_start);
        delay = delay > 33 ? 0 : delay;
        SDL_Delay(delay);
    }

    return SUCCESS;
}

int game_init_assets(void)
{
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

void game_process_input(void)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
    case SDL_QUIT: {
        game->is_running = false;
    } break;

    case SDL_KEYDOWN: {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            game->is_running = false;
        }

        if (event.key.keysym.sym == SDLK_RIGHT) {
            game->scroll_x = 15;
        }

        if (event.key.keysym.sym == SDLK_LEFT) {
            game->scroll_x = -15;
        }

        if (event.key.keysym.sym == SDLK_DOWN) {
            game->cur_level++;
        }

        if (event.key.keysym.sym == SDLK_UP) {
            game->cur_level--;
        }
    } break;
    }
}

void game_update(void)
{
    if (game->cur_level == 0xff) {
        game->cur_level = 0;
    }
    if (game->cur_level > 9) {
        game->cur_level = 9;
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

void game_render(void)
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    game_render_world();
    game_render_player();

    SDL_RenderPresent(renderer);
}

void game_render_world(void)
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

void game_render_player(void)
{
    SDL_Rect dest = {
        .x = game->player_px,
        .y = game->player_py,
        .w = PLAYER_W,
        .h = PLAYER_H,
    };
    SDL_RenderCopy(renderer, assets->gfx_tiles[PLAYER_TILE], NULL, &dest);
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

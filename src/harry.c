#include "harry.h"
#include "error.h"
#include <stdlib.h>

static struct game_state_t *game;
static struct game_assets_t *assets;
SDL_Window *window;
SDL_Renderer *renderer;

int game_init(void)
{
    FILE *fd_level;
    char fname[16];
    char file_num[4];

    game = malloc(sizeof(struct game_state_t));
    if (!game) {
        fprintf(stderr, "Error allocating memory for game state.\n");
        return ERR_ALLOC;
    }

    game->is_running = false;
    game->view_x = 0;
    game->view_y = 0;
    game->cur_level = 0;

    for (int i = 0; i < 10; i++) {
        fname[0] = '\0';
        strcat(fname, "res/level");
        sprintf(&file_num[0], "%u", i);
        strcat(fname, file_num);
        strcat(fname, ".dat");

        fd_level = fopen(fname, "rb");
        if (!fd_level) {
            // TODO(claude): return error
            fprintf(stderr, "Error opening file %s\n", fname);
            return ERR_OPENING_FILE;
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL init error: %s\n", SDL_GetError());
        return ERR_SDL_INIT;
    }

    SDL_Window *window;
    SDL_Renderer *renderer;

    if (SDL_CreateWindowAndRenderer(320 * DISPLAY_SCALE, 200 * DISPLAY_SCALE, 0, &window, &renderer) != 0) {
        SDL_Log("SDL window/renderer error: %s\n", SDL_GetError());
        return ERR_SDL_CREATE_WIN_RENDER;
    }

    SDL_RenderSetScale(renderer, DISPLAY_SCALE, DISPLAY_SCALE);

    assets = malloc(sizeof(struct game_assets_t));
    if (!assets) {
        fprintf(stderr, "Error allocating memory for game assets.\n");
        return ERR_ALLOC;
    }
    game_init_assets();

    game->is_running = true;

    return SUCCESS;
}

int game_run(void)
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    while (game->is_running) {
        game_process_input();
        game_update();
        game_render();
    }

    return SUCCESS;
}

void game_init_assets(void) {}

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
    } break;
    }
}

void game_update(void) {}

void game_render(void) {}

int game_destroy(void)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    free(assets);
    free(game);

    return SUCCESS;
}

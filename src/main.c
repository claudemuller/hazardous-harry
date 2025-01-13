#define SDL_MAIN_HANDLED

#include "error.h"
#include "game.h"
#include "log.h"
#include <string.h>

// TODO:(lukefilewalker) remove res dir from gitignore when new assets have been created!

int main(int argc, char *argv[])
{
    bool debug = false;
    if (argc > 1) {
        if (strncmp(argv[1], "--debug", strlen("--debug")) == 0) {
            log_visibility(LOG_DEBUG);
            debug = true;
        }
    }

    err_handle(game_init(debug));
    err_handle(game_run());
    err_handle(game_destroy());

    return 0;
}

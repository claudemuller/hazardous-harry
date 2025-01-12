#include <string.h>
#define SDL_MAIN_HANDLED

#include "error.h"
#include "harry.h"
#include "log.h"

int main(int argc, char *argv[])
{
    // TODO(claude): should start off false
    bool debug = true;

    if (argc > 1) {
        if (strcmp(argv[1], "debug")) {
            log_visibility(LOG_DEBUG);
            debug = true;
        }
    }

    // TODO(claude): remove this in favour of cmd args
    log_visibility(LOG_DEBUG);

    err_handle(game_init(debug));
    err_handle(game_run());
    err_handle(game_destroy());

    return 0;
}

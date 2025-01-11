#define SDL_MAIN_HANDLED

#include "error.h"
#include "harry.h"
#include "log.h"

int main(void)
{
    log_visibility(LOG_DEBUG);

    err_handle(game_init());
    err_handle(game_run());
    err_handle(game_destroy());

    return 0;
}

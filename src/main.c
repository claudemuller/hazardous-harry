#include "error.h"
#include "harry.h"

int main(void)
{
    err_handle(game_init());
    err_handle(game_run());
    err_handle(game_destroy());

    return 0;
}

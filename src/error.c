#include "error.h"
#include <stdio.h>
#include <stdlib.h>

void err_handle(int err)
{
    if (err != SUCCESS) {
        fprintf(stderr, "msg");
        exit(1);
    }
    // fprintf(stderr, "Error allocating memory for game assets.\n");
    // return 1;
}

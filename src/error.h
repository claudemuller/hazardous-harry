#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char err_additional[256] = {0};

enum {
    SUCCESS,
    ERR_ALLOC,
    ERR_OPENING_FILE,
    ERR_SDL_INIT,
    ERR_SDL_CREATE_WIN_RENDER,
};

static const char *err_messages[] = {
    "",
    "Error allocating memory",
    "Error opening file",
    "Error initialising SDL",
    "Error creating SDL window/renderer",
};

static inline void err_handle(const int err)
{
    if (err != SUCCESS) {
        // TODO(claude): handle error types e.g. fatal etc.
        fprintf(stderr, "\033[1;31m%s: %s\033[0m\n", err_messages[err], err_additional);
        exit(1);
    }
}

static inline int err_fatal(const int err, const char *msg)
{
    strncpy(err_additional, msg, strlen(msg));
    return err;
}

#endif // !ERROR_H

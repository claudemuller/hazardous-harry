#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char err_additional[256] = {0};

const char *err_messages[] = {
    "",
    "Error allocating memory",
    "Error opening file",
    "Error initialising SDL",
    "Error creating SDL window/renderer",
    "Error loading BMP",
    "Error initialising fonts",
    "Error loading font",
};

void err_handle(const int err)
{
    if (err != SUCCESS) {
        // TODO:(lukefilewalker): handle error types e.g. fatal etc.
        fprintf(stderr, "\033[1;31m🚨 %s: %s\033[0m\n", err_messages[err], err_additional);
        exit(1);
    }
}

// TODO:(lukefilewalker): make this variadic after int err
int err_fatal(const int err, const char *msg)
{
    if (msg) {
        strncpy(err_additional, msg, strlen(msg));
    }

    return err;
}

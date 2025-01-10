#include "log.h"
#include <stdio.h>
#include <time.h>

int log_level;

void log_visibility(const int level) { log_level = level; }

void log_info(const char *tag, const char *msg)
{
    if (log_level == LOG_DEBUG) {
        char buffer[20];
        time_t now = time(NULL);
        struct tm *local = localtime(&now);

        strftime(buffer, sizeof(buffer), "%Y:%m:%d:%H.%M.%S", local);
        fprintf(stdout, "\033[37mℹ️ %s: [%s] %s\033[0m\n", buffer, tag, msg);
    }
}

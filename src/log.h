#ifndef LOG_H
#define LOG_H

#include <stdbool.h>
#include <time.h>

#define LOG_INFO(tag, ...)                                                                                             \
    char buffer[20];                                                                                                   \
    time_t now = time(NULL);                                                                                           \
    struct tm *local = localtime(&now);                                                                                \
    strftime(buffer, sizeof(buffer), "%Y:%m:%d:%H.%M.%S", local);                                                      \
    fprintf(stdout, "\033[37mℹ️ %s: [%s] ", buffer, tag);                                                               \
    fprintf(stdout, __VA_ARGS__);                                                                                      \
    fprintf(stdout, "\033[0m\n")

enum {
    LOG_DEBUG = 1,
};

extern int log_level;

void log_visibility(const int);
void log_info(const char *, const char *);

#endif // !LOG_H

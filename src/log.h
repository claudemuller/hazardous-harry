#ifndef LOG_H
#define LOG_H

#include <stdbool.h>

enum {
    LOG_DEBUG = 1,
};

extern int log_level;

void log_visibility(const int);
void log_info(const char *, const char *);

#endif // !LOG_H

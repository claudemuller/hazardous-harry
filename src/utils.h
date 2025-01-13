#ifndef HH_UTILS_H
#define HH_UTILS_H

#include <stdio.h>

void itoa(int value, char *str, int base)
{
    if (base == 10) {
        snprintf(str, 12, "%d", value);
    } else if (base == 16) {
        snprintf(str, 12, "%x", value);
    } else if (base == 8) {
        snprintf(str, 12, "%o", value);
    } else {
        // Unsupported base
        str[0] = '\0';
    }
}

#endif // !HH_UTILS_H

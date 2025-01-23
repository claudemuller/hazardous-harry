#ifndef HH_UTILS_H
#define HH_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef _WIN32
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
#endif

bool in_array(const uint8_t *haystack, const uint8_t needle, const size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (haystack[i] == needle) {
            return haystack[i];
        }
    }
    return false;
}

#endif // !HH_UTILS_H

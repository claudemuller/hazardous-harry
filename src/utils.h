#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
void itoa(int value, char *str, int base)
{
    if (base == 10) {
        snprintf(str, 12, "%d", value); // Base 10 for decimal
    } else if (base == 16) {
        snprintf(str, 12, "%x", value); // Base 16 for hexadecimal
    } else if (base == 8) {
        snprintf(str, 12, "%o", value); // Base 8 for octal
    } else {
        // Unsupported base
        str[0] = '\0';
    }
}

#endif // !UTILS_H

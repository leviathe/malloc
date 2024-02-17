#include "alignment.h"

#include <stdio.h>

size_t align(size_t size)
{
    size_t len = sizeof(long double);
    if (size % len)
    {
        size_t res = len - (size % len);
        size_t res2;
        if (__builtin_add_overflow(len, size, &res2))
            return 0;
        return size + res;
    }
    else
        return size;
}

#include "beware_overflow.h"

void *beware_overflow(void *ptr, size_t nmemb, size_t size)
{
    char *ptr2 = ptr;
    size_t res;
    if (__builtin_mul_overflow(nmemb, size, &res))
        return NULL;
    ptr2 += res;
    return ptr2;
}

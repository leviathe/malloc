#include "page_begin.h"

void *page_begin(void *ptr, size_t page_size)
{
    size_t p = (size_t)ptr;
    size_t new_add = (p & ~(page_size - 1));
    char *res = 0;
    for (size_t i = 0; i < new_add; i += page_size)
    {
        res += page_size;
    }
    return res;
}

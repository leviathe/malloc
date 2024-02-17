#include "recycler.h"

#include <stdlib.h>

struct recycler *recycler_create(size_t block_size, size_t total_size)
{
    if ((block_size % sizeof(size_t)) || block_size == 0 || total_size == 0
        || (total_size % block_size))
        return NULL;
    struct recycler *p = malloc(sizeof(struct recycler));
    if (p == NULL)
        return NULL;
    void *p2 = malloc(total_size);
    if (p2 == NULL)
    {
        free(p);
        return NULL;
    }
    p->block_size = block_size;
    p->capacity = total_size / block_size;
    p->chunk = p2;
    struct free_list *tmp = p->chunk;
    p->free = tmp;
    for (size_t i = 0; i < p->capacity - 1; i++)
    {
        tmp->next = tmp + block_size / sizeof(struct free_list);
        tmp = tmp->next;
    }
    tmp->next = NULL;
    return p;
}

void recycler_destroy(struct recycler *r)
{
    if (r)
    {
        free(r->chunk);
        free(r);
    }
}

void *recycler_allocate(struct recycler *r)
{
    if (r == NULL || r->free == NULL)
        return NULL;
    struct free_list *p = r->free;
    r->free = p->next;
    return p;
}

void recycler_free(struct recycler *r, void *block)
{
    if (r != NULL || block != NULL)
    {
        struct free_list *p = block;
        p->next = r->free;
        r->free = p;
    }
}

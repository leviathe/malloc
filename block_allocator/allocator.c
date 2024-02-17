#include "allocator.h"

#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

struct blk_allocator *blka_new(void)
{
    struct blk_allocator *a = malloc(sizeof(struct blk_allocator));
    if (a)
        a->meta = NULL;
    return a;
}

struct blk_meta *blka_alloc(struct blk_allocator *blka, size_t size)
{
    int par1 = PROT_READ | PROT_WRITE;
    int par2 = MAP_PRIVATE | MAP_ANONYMOUS;
    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t total_struct_size = size * sizeof(char);
    size_t intruder = sizeof(size_t) + sizeof(struct blk_meta *);
    total_struct_size += intruder;
    size_t nb_page = total_struct_size / page_size;
    nb_page += total_struct_size % page_size != 0 ? 1 : 0;
    size_t total_size = nb_page * page_size;
    struct blk_meta *m = mmap(NULL, total_size, par1, par2, -1, 0);
    if (m)
    {
        m->size = total_size - intruder;
        m->next = blka->meta;
        blka->meta = m;
    }
    return m;
}

void blka_free(struct blk_meta *blk)
{
    if (blk)
    {
        size_t page_size = sysconf(_SC_PAGESIZE);
        size_t total_size = blk->size * sizeof(char) + sizeof(size_t);
        total_size += sizeof(struct blk_meta *);
        long nb_page = total_size / page_size;
        nb_page += total_size % page_size ? 1 : 0;
        munmap(blk, nb_page * page_size);
    }
}

void blka_pop(struct blk_allocator *blka)
{
    if (blka)
    {
        struct blk_meta *m = blka->meta;
        if (m)
        {
            blka->meta = blka->meta->next;
            blka_free(m);
        }
    }
}

void blka_delete(struct blk_allocator *blka)
{
    if (blka)
    {
        while (blka->meta)
        {
            blka_pop(blka);
        }
        free(blka);
    }
}

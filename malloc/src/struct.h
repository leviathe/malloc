#ifndef STRUCT_H
#define STRUCT_H

#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifdef DEBUG
#    include <stdio.h>
#    define malloc my_malloc
#    define realloc my_realloc
#    define free my_free
#    define calloc my_calloc
#endif /* !DEBUG */

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t number, size_t size);
void *realloc(void *ptr, size_t size);

struct data
{
    struct data *prev;
    struct data *next;
    size_t size;
    char status;
};

struct free_list
{
    struct free_list *prev;
    struct free_list *next;
};

struct page
{
    struct page *prev;
    struct page *next;
    size_t size;
    size_t padding;
};

struct var_global
{
    struct page *page;
    struct free_list *list;
    long page_size;
};

#endif /* !STRUCT_H */

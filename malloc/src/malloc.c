#include "struct.h"

#define DATA sizeof(struct data)
#define FREE sizeof(struct free_list)
#define PAGE sizeof(struct page)

struct var_global global;

//---------------------MALLOC---------------------
// Init global global variable
static void init_global(void)
{
    global.page = NULL;
    global.list = NULL;
    global.page_size = sysconf(_SC_PAGE_SIZE);
}

// Convert void * en size_t
static size_t void_to_size(void *p)
{
    return (size_t)p;
}

// Convert size_t en void *
static void *size_to_void(size_t p)
{
    return (void *)p;
}

// Data alignment
static size_t align(size_t size)
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

// Get the beginning of a page
void *page_begin(void *ptr)
{
    size_t p = void_to_size(ptr);
    p = (p & ~(global.page_size - 1));
    return size_to_void(p);
}

// Add pages in the global variable
static void add_page(void *ptr)
{
    struct page *p = ptr;
    if (global.page != NULL)
        global.page->prev = p;
    p->prev = NULL;
    p->next = global.page;
    global.page = p;
}

// Create and intitialize one page then stock it in global var and return the
// address
static void *create_page(size_t nb_page)
{
    long page_size = sysconf(_SC_PAGESIZE);
    int param1 = PROT_READ | PROT_WRITE;
    int param2 = MAP_PRIVATE | MAP_ANONYMOUS;
    void *addr = mmap(NULL, page_size * nb_page, param1, param2, -1, 0);
    if (addr == MAP_FAILED)
        return NULL;
    struct page *p = addr;
    p->padding = 0;
    p->size = 0;
    add_page(p);
    return p;
}

// Init data header
static void init_data(struct data *d1, struct data *d2, size_t size)
{
    d1->prev = d2;
    if (d2)
        d2->next = d1;
    d1->next = NULL;
    d1->size = size;
    d1->status = 0;
}

// Create data header set to freed and return the next address
static void *create_data(void *p, size_t size)
{
    struct page *p1 = p;
    struct data *d = size_to_void(void_to_size(p1 + 1));
    init_data(d, NULL, size);
    return d;
}

// Init free_list and add it in the global var
static void init_n_push_free(struct free_list *f)
{
    f->prev = NULL;
    f->next = global.list;
    if (global.list != NULL)
        global.list->prev = f;
    global.list = f;
}

// Create free_list header, stock it in the free_list and return the next
// address
static void *create_free(void *p)
{
    struct data *p1 = p;
    struct free_list *f = size_to_void(void_to_size(p1 + 1));
    init_n_push_free(f);
    return f;
}

// Get the number of page for data
size_t get_nb_page(size_t data)
{
    size_t size = data + PAGE + FREE + DATA;
    size_t nb_page = size / global.page_size;
    nb_page += data % global.page_size ? 1 : 0;
    return nb_page;
}

// Initialise pages with headers return if errors else the pointers to free_list
static void *init_page(size_t data)
{
    struct page *p1 = create_page(get_nb_page(data));
    if (p1)
    {
        size_t occupied = PAGE + FREE + DATA;
        struct data *p2 = create_data(p1, global.page_size - occupied);
        return create_free(p2);
    }
    return NULL;
}

// Remove free_list from global var
static void pop_free(struct free_list *f)
{
    if (f->next)
        f->next->prev = f->prev;
    if (f->prev)
        f->prev->next = f->next;
    else
        global.list = f->next;
    f->prev = NULL;
    f->next = NULL;
}

// Update data headers and create free headers if possible
static void *set_unfree(void *ptr, size_t data, int end)
{
    struct free_list *f = ptr;
    pop_free(f);
    struct data *d = size_to_void(void_to_size(f - (DATA / FREE)));
    size_t tmp = d->size;
    d->size = data;
    d->status = 1;
    struct page *p = page_begin(ptr);
    p->size += 1;
    if (end)
    {
        tmp -= data + FREE + DATA;
        struct data *d2 =
            size_to_void(void_to_size(f + ((FREE + data) / FREE)));
        init_data(d2, d, tmp);
        struct free_list *f2 = size_to_void(void_to_size(d2 + 1));
        init_n_push_free(f2);
    }
    return f + 1;
}

// Get header data
static size_t get_data_size(void *f)
{
    struct data *d = f;
    d -= 1;
    return d->size;
}

// Search free place with right data size in free_list
static void *search(size_t size)
{
    struct free_list *f = global.list;
    while (f && get_data_size(f) < size)
        f = f->next;
    if (f == NULL)
        return NULL;
    return f;
}

// Check if it is possible to add empty space
static int i_t_e_p(void *f, size_t data)
{
    size_t size = get_data_size(f);
    return data + FREE + DATA + sizeof(long double) <= size;
}

// Check if it is big allocation
static int one_page(size_t data)
{
    return get_nb_page(data) == 1;
}

// Return if create a new free bock is possible
static int is_end(void *f, size_t data)
{
    return i_t_e_p(f, data) && one_page(data);
}

// Create, init page and return
static void *return_page(size_t data)
{
    void *f = init_page(data);
    if (f == NULL)
        return NULL;
    return set_unfree(f, data, is_end(f, data));
}

//---------------------PRETTY PRINT---------------------
#ifdef DEBUG

static void *to_void(void *p)
{
    return p;
}

void print_data(struct data *d, struct free_list *f)
{
    printf("┇ data address = %p\n", to_void(d));
    printf("┇ data prev = %p\n", to_void(d->prev));
    printf("┇ data next = %p\n", to_void(d->next));
    printf("┇ data size = %lu\n", d->size);
    printf("┇ data status = %u\n", d->status);
    printf("┇ free address = %p\n", to_void(f));
    printf("┇ free prev = %p\n", to_void(f->prev));
    printf("┇ free next = %p\n\n", to_void(f->next));
}

void print_page(void)
{
    for (struct page *p = global.page; p != NULL; p = p->next)
    {
        printf("--------PAGE--------\n");
        printf("┇ page address = %p\n", to_void(p));
        printf("┇ page prev = %p\n", to_void(p->prev));
        printf("┇ page next = %p\n", to_void(p->next));
        printf("┇ page size = %lu\n", p->size);
        printf("┇ page padding = %lu\n\n", p->padding);
        struct data *d = size_to_void(void_to_size(p + 1));
        while (d != NULL)
        {
            struct free_list *f = size_to_void(void_to_size(d + 1));
            print_data(d, f);
            d = d->next;
        }

        printf("┇ global page = %p\n", to_void(global.page));
        printf("┇ global list = %p\n", to_void(global.list));
        printf("--------------------\n");
    }
}
#endif /* !DEBUG */

__attribute__((visibility("default"))) void *malloc(size_t size)
{
#ifdef DEBUG
    printf("--------MALLOC--------\n");
#endif /* !DEBUG */
    size_t data = align(size);
    if (global.page_size != sysconf(_SC_PAGE_SIZE))
        init_global();
    if (global.list == NULL)
    {
        void *res = return_page(data);
#ifdef DEBUG
        print_page();
#endif /* !DEBUG */
        return res;
    }
    else
    {
        void *f = search(data);
        if (f)
        {
            void *res = set_unfree(f, data, is_end(f, data));
#ifdef DEBUG
            print_page();
#endif /* !DEBUG */
            return res;
        }
        else
        {
            void *res = return_page(data);
#ifdef DEBUG
            print_page();
#endif /* !DEBUG */
            return res;
        }
    }
}

//---------------------FREE---------------------
// Get the free pointer from data pointer
static void *get_free(void *ptr)
{
    struct free_list *f = ptr;
    f -= 1;
    return f;
}

// Get the free pointer from data pointer
static void *get_data(void *f)
{
    struct data *d = f;
    d -= 1;
    return d;
}

// Set the data block free and return the data header pointer
static void set_free(struct page *p, struct data *d, struct free_list *f)
{
    p->size -= 1;
    d->status = 0;
    init_n_push_free(f);
}

// Merge two data blocks
static void merge(void *ptr1, void *ptr2)
{
    struct data *d1 = ptr1;
    struct data *d2 = ptr2;
    struct free_list *f2 = size_to_void(void_to_size(ptr2) + DATA);
    pop_free(f2);
    d1->size += d2->size + DATA + FREE;
    d1->next = d2->next;
    if (d1->next != NULL)
        d1->next->prev = d1;
}

// Remove page from global var
static void pop_page(struct page *p)
{
    if (p->next)
        p->next->prev = p->prev;
    if (p->prev)
        p->prev->next = p->next;
    else
        global.page = p->next;
    p->prev = NULL;
    p->next = NULL;
}

// Unmap page
static int free_page(void *ptr)
{
    struct page *p = ptr;
    struct data *d = size_to_void(void_to_size(p + 1));
    struct free_list *f = size_to_void(void_to_size(d + 1));
    pop_free(f);
    pop_page(p);
    size_t size = d->size + PAGE + DATA + FREE;
    int res = munmap(p, size);
    return res;
}

__attribute__((visibility("default"))) void free(void *ptr)
{
#ifdef DEBUG
    printf("--------FREE--------\n");
#endif /* !DEBUG */
    if (ptr != NULL)
    {
        struct free_list *f = get_free(ptr);
        struct data *d = get_data(f);
        struct page *p = page_begin(ptr);
        set_free(p, d, f);
        if (d->next != NULL && d->next->status == 0)
            merge(d, d->next);
        if (d->prev != NULL && d->prev->status == 0)
            merge(d->prev, d);
        if (p->size == 0)
            free_page(p);
#ifdef DEBUG
        print_page();
#endif /* !DEBUG */
    }
}

//---------------------REALLOC---------------------
__attribute__((visibility("default"))) void *realloc(void *ptr, size_t size)
{
#ifdef DEBUG
    printf("--------REALLOC--------\n");
#endif /* !DEBUG */
    if (size == 0 && ptr != NULL)
    {
        free(ptr);
        return NULL;
    }
    else if (ptr != NULL)
    {
        char *p = ptr;
        size_t data_size = get_data_size(p - (FREE / sizeof(char)));
        void *ptr2 = malloc(size);
        if (data_size <= size)
            ptr2 = memcpy(ptr2, ptr, data_size);
        else
            ptr2 = memcpy(ptr2, ptr, size);
        free(p);
        return ptr2;
    }
    else
    {
        void *res = malloc(size);
        return res;
    }
}

//---------------------CALLOC---------------------
// Check overflow
static int is_overflow(size_t nmemb, size_t size)
{
    size_t res;
    if (__builtin_mul_overflow(nmemb, size, &res))
        return 1;
    return 0;
}

__attribute__((visibility("default"))) void *calloc(size_t nmemb, size_t size)
{
#ifdef DEBUG
    printf("--------CALLOC--------\n");
#endif /* !DEBUG */
    if (is_overflow(nmemb, size))
        return NULL;
    char *p = malloc(nmemb * size);
    size_t data_size = get_data_size(p - ((FREE) / sizeof(char)));
    p = memset(p, 0, data_size);
    return p;
}

#include "../src/struct.h"

int main(void)
{
    char *c1 = calloc(7, sizeof(int));
    char *c2 = malloc(2000);
    c1 = realloc(c1, 10000);
    free(c1);
    free(c2);
    return 0;
}

#include "hallocator.h"
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
    int *a, *b;

    a = mhalloc(30 * sizeof(int));

    for (int i = 0; i < 30; i++)
        a[i] = i;

    a = rehalloc(a, 100* sizeof(int));

    for (int i = 30; i < 100; i++)
        a[i] = i;

    for (int i = 0; i < 100; i++)
        printf("%d, ", a[i]);
    puts("");

    print_mem_map();


    return 0;
}

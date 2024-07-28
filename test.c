#include "hallocator.h"
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
    int *a, *b;
    b = mhalloc(100);
    a = mhalloc(30);
    rehalloc(a, 100);
    print_mem_map();


    return 0;
}

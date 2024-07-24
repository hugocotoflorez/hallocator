#include "hallocator.h"
#include <unistd.h>

int
main(int argc, char *argv[])
{
    char *a, *b;
    halloc_init();
    print_mem_map();
    a = mhalloc(100);
    print_mem_map();
    b = mhalloc(100);
    print_mem_map();
    fhree(a);
    print_mem_map();
    fhree(b);
    print_mem_map();

    return 0;
}

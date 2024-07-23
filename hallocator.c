#include "hallocator.h"
#include <sys/mman.h>

void
halloc_init()
{
    // mmap() return a chunk of free space
    node_t *head =
    mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

    head->size = SIZE - sizeof(node_t);
    head->next = NULL;
}

void
fhree(void *ptr)
{
    header_t *hptr = (header_t *) ptr - 1;
}

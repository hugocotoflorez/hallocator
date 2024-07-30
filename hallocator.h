#ifndef _HALLOCATOR
#define _HALLOCATOR

/*
 * Memory management library
 * Something as in stdlib
 *
 * Author: Hugo Coto Florez
 *
 * Inspiration: ostep book
 */

// total chunk size
#define SIZE 4096

#define MAGIC 0x777

typedef struct __node_t
{
    int              size;
    struct __node_t *next;
} node_t;

typedef struct
{
    int size;
    int magic;
} header_t;

/*
 * void *mhalloc(int size)
 * Alloc 'size' bytes and return a pointer to the first byte
 * If cant alloc, return NULL
 */
void *
mhalloc(int size);

/*
 * void *rehalloc(void* ptr, int size)
 * Alloc 'size' bytes and move data from ptr to a new possition,
 * returns a pointer to a new possition and free previous pointer.
 * On error previous pointer is not free and return NULL
 */
void *
rehalloc(void *ptr, int size);

/*
 * void fhree(void* ptr)
 * Free memory pointed by ptr
 */
void
fhree(void *ptr);


void
print_mem_map(void** ptrs);

/*
 * Automatically initialized before main(), can also be
 * called, it checks if it was initialized before
 */
void
halloc_init();

/*
 * Automatically call at exit, can also be called manually
 * but calls to allocator after this function would end in a segfault
 */
void
halloc_destroy();

#endif // !_HALLOCATOR

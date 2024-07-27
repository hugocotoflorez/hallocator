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

// define null if not defined
#ifndef NULL
#define NULL (void *) 0
#endif

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
 * void* mhalloc(int size)
 * Alloc 'size' bytes and return a pointer to the first byte
 * If cant alloc it return NULL
 */
void *
mhalloc(int size);

/*
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
print_mem_map();

/*
 * Automatically initialized before main(), can also be
 * called, it checks if it was initialized before
 */
void
halloc_init();

#endif // !_HALLOCATOR

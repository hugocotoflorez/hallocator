#include "hallocator.h"
#include <stdio.h>    // puts, printf
#include <stdlib.h>   // atexit
#include <sys/mman.h> // mmap, munmap


// global reference to free list (sorted linked list started at head)
node_t *head = NULL;


void
halloc_destroy()
{
    munmap(head, SIZE);
}

// Automatically initialized before main(), can also be
// called manually
__attribute__((constructor)) void // call function before main
halloc_init()
{
    // avoid multiple initializations
    if (head == NULL)
    {
        // mmap() return a block of free space
        head =
        mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

        head->size = SIZE - sizeof(node_t);
        head->next = NULL;

        atexit(halloc_destroy);
    }
}

void
blockcpy(void *ptr_dest, void *ptr_src, int block_size)
{
    int regions;
    // regions of size 'sizeof(...)' in ptr
    regions = block_size / sizeof(long long int);

    // copy chunks of data
    for (int i = 0; i < regions; i++)
        ((long long int *) ptr_dest)[i] = ((long long int *) ptr_src)[i];

    // copy remaining bytes from out of last chunk
    for (int i = 0; i < block_size % sizeof(long long int); i++)
        ((char *) ptr_dest + regions * sizeof(long long int))[i] =
        ((char *) ptr_src + regions * sizeof(long long int))[i];
}

void
fhree(void *ptr)
{
    header_t *hptr = (header_t *) ptr - 1;
    // I think the following check never take place, so removed
    // hptr->size <= 0 ||
    if (hptr->magic != MAGIC)
    {
#ifdef VERBOSE
        puts("\e[31mfhree: invalid pointer\e[0m");
#endif
        return;
    }
    // unset header pointer as valid block pointer
    hptr->magic = 0;

    // get next
    node_t *next; // pointer to next free block header
    node_t *temp;

    // insert new node into list in O(n)
    temp = head;
    do
    {
        if ((void *) temp->next > ptr || temp->next == NULL)
        {
            next       = temp->next;
            temp->next = (void *) hptr;
            break;
        }
        temp = temp->next;

    } while (temp != NULL);

    // check if new free block is between two free blocks
    if ((void *) hptr + hptr->size + sizeof(header_t) == next &&
        (void *) hptr + hptr->size + sizeof(header_t) == next)
    {
        temp->next = next->next;
        temp->size += next->size + sizeof(header_t) + sizeof(node_t) + hptr->size;
    }
    // check if next free junk is just after new one
    else if ((void *) hptr + hptr->size + sizeof(header_t) == next)
    {
        *((node_t *) hptr) = (node_t){
            .size = hptr->size + sizeof(header_t) + next->size,
            .next = next->next,
        };
    }
    // check if previous free block is just before new one
    else if ((void *) temp + temp->size + sizeof(node_t) == hptr)
    {
        temp->size += hptr->size + sizeof(header_t);
        temp->next = next;
    }
    // current block is not close to another free block
    else
    {
        // create node at head pointer
        *((node_t *) hptr) = (node_t){
            .size = hptr->size + sizeof(header_t) - sizeof(node_t),
            .next = next,
        };
    }
}

void *
mhalloc(int size)
{
    node_t *temp      = head;
    void   *ptr       = NULL;
    int     best_size = SIZE;

    // check if it is initialized
    if (head == NULL)
    {
#ifdef VERBOSE
        puts("\e[31hallocator: not initialized correctly\e[0m");
#endif
        return NULL;
    }

    // check if size is valid
    if (size <= 0)
    {
#ifdef VERBOSE
        puts("\e[31mmhalloc: invalid size\e[0m");
#endif
        return NULL;
    }

    // if size is smaller than headers diff new header dont have enought space at free
    if (size < sizeof(node_t) - sizeof(header_t))
    {
        size = sizeof(node_t) - sizeof(header_t);
    }

    // best fit algorithm (smallest free space greater or equal than size)
    // done in a linear way, end in O(n) algorithm
    do
    {
        if (temp->size >= size && best_size > temp->size)
        {
            best_size = temp->size;
            ptr = (void *) temp; // ptr is the list entry that best fit
        }
        temp = temp->next;

    } while (temp != NULL);
    // if pointer fits in memory
    if (ptr != NULL)
    {
        // update free list
        ((node_t *) ptr)->size -= size + sizeof(header_t);
        // move pointer to last free 'size' bytes in free space
        ptr += best_size - size + sizeof(node_t);
        // add header to allocated memory
        *((header_t *) ptr - 1) = (header_t){ .size = size, .magic = MAGIC };
    }
    // return ptr or NULL if cant allocate that memory ammount
    return ptr;
}


void *
rehalloc_after(void *ptr, int size, header_t *hptr)
{
    node_t *next    = ptr + hptr->size;
    int     newsize = size - hptr->size;

    // move next node
    *(node_t *) (next + newsize) = (node_t){
        .size = next->size - newsize,
        .next = next->next,
    };

    // TODO

    return NULL;
}

void *
rehalloc_before(void *ptr, int size, header_t *hptr)
{
    node_t *prev = head;
    while (prev != NULL)
    {
        if ((void *) prev->next > ptr)
            break;
    }
    // check if previous free block is just before current block
    if ((void *) prev + prev->size == hptr)
    {
    }

    // TODO

    return NULL;
}

void *
rehalloc(void *ptr, int size)
{
    void     *newptr = NULL;
    header_t *hptr;

    if (head == NULL) // check if it is initialized
        exit(EXIT_FAILURE);

    if (ptr == NULL) // allow use rehalloc as mhalloc
        return mhalloc(size);

    hptr = ptr - sizeof(header_t); // get header

    if (hptr->size >= size) // if size is invalid return NULL
        return NULL;

    // try to allocate just after
    if ((newptr = rehalloc_after(ptr, size, hptr)) != NULL)
        return newptr;

    // try to allocate just before
    if ((newptr = rehalloc_before(ptr, size, hptr)) != NULL)
        return newptr;

    // allocate new block and move data, then free current block
    if ((newptr = mhalloc(size)) == NULL) // check for out-of-memmory error
        return NULL;

    blockcpy(newptr, ptr, hptr->size); // move data
    fhree(ptr);

    return newptr;
}

// si hay free en el medio no va
void
print_mem_map()
{
    node_t   *temp = head;
    header_t *temp2;
    puts("+---------------------+");
    do
    {
        printf("|size: %14d | %p\n", temp->size, temp);
        printf("|next: %14p |\n", temp->next);
        puts("+---------------------+");

        if (temp->size > 0)
        {
            puts("|free                 |");
            puts("+---------------------+");
        }

        temp2 = (void *) temp + temp->size + sizeof(node_t);

        while ((void *) temp2 != (void *) temp->next &&
               (void *) temp2 < (void *) head + SIZE)
        {
            if (temp2->magic == MAGIC)
            {
                printf("|size: %14d | %p\n", temp2->size, temp2);
                printf("|magic: %13x |\n", temp2->magic);
                puts("+---------------------+");
                puts("|used                 |");
                puts("+---------------------+");
                temp2 = (void *) temp2 + temp2->size + sizeof(header_t);
            }
            else
            {
                printf("|size: %14d | %p\n", ((node_t *) temp2)->size, temp2);
                printf("|next: %14p |\n", ((node_t *) temp2)->next);
                puts("+---------------------+");
                puts("|free                 |");
                puts("+---------------------+");
                temp2 = (void *) temp2 + temp2->size + sizeof(node_t);
            }
        }
        temp = temp->next;

    } while (temp != NULL);
}

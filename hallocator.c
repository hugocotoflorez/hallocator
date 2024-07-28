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
__move_data(void *ptr_dest, void *ptr_src, int block_size)
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

#define diff (size - hptr->size)

void *
rehalloc(void *ptr, int size)
{
    void     *newptr = NULL;
    node_t   *temp   = head;
    node_t   *prev   = head;
    header_t *hptr;
    header_t *newhptr;
    header_t *next;

    // check if it is initialized
    if (head == NULL)
    {
#ifdef VERBOSE
        puts("\e[31hallocator: not initialized correctly\e[0m");
#endif
        return NULL;
    }

    // allow use rehalloc as mhalloc
    if (ptr == NULL)
        return mhalloc(size);

    // get header
    hptr = ptr - sizeof(header_t);

    // if size is invalid returns the same pointer
    if (hptr->size >= size)
    {
#ifdef VERBOSE
        puts("\e[31rehalloc: Invalid new size\e[0m");
#endif
        return newptr;
    }

    // check if just after this block there are a free block
    next = (void *) hptr + hptr->size + sizeof(header_t);

    if (next->magic != MAGIC) // if it is equal to magic is used
    {
        // check if it is big enought
        if (next->size + sizeof(node_t) >= size - hptr->size)
        {
            hptr->size += size; // update new size
            next->size -= size; // update next block size

            // update previous node next-pointer
            while (temp->next != NULL)
            {
                if (temp->next == (node_t *) next)
                {
                    temp->next = (void *) next + size;
                    break;
                }
                temp = temp->next;
            }

            // move next-node 'size' bytes
            *(node_t *) ((void *) next + size) = *(node_t *) next;

            // dont have to move any data
            // return same pointer as it dont have to move
            return ptr;
        }
    }
    // get previous node. lineal search, inefficient
    while ((void *) prev->next < ptr && prev->next != NULL)
        prev = prev->next;

    // check is next used block after previous free block is current used block
    if ((void *) prev + prev->size + sizeof(node_t) == hptr)
    {
        if (prev->size > diff) // TODO: puede ser mas grande y que quepa
                               // entero usando todo el bloque anterior
        {
            prev->size -= diff;
            // move pointer to make space to new data
            newhptr  = (void *) hptr - diff;
            *newhptr = (header_t){
                .size  = size,
                .magic = MAGIC,
            };

            newptr = (void *) newhptr + sizeof(header_t);

            // move data to new position
            __move_data(newptr, ptr, hptr->size);

            return newptr;
        }
    }

    // allocate new block and move data, then free current block
    newptr = mhalloc(size);

    // check for out-of-memmory error
    if (newptr == NULL)
        return NULL;

    // move data
    __move_data(newptr, ptr, hptr->size);
    fhree(ptr);

    return newptr;
}
#undef diff

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

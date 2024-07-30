#include "hallocator.h"
#include <stdio.h>  // puts, printf
#include <stdlib.h> // atexit
#include <string.h>
#include <sys/mman.h> // mmap, munmap
#include <sys/types.h>
#include <unistd.h> // read

/*
 *  Important message for readers
 * Void* arithmetic is illegal in C
 * but GCC allow it giving void* size 1
 * So compile this code with other editor
 * may produce errors.
 *  Solution:
 *  change void* to char* on arithmetic
 */

// global reference to free list (sorted linked list started at head)
node_t *head = NULL;


void
halloc_destroy()
{
    munmap(head, SIZE);
}

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
blockcpy2(void *ptr_dest, void *ptr_src, int block_size)
{
    memmove(ptr_dest, ptr_src, block_size);
}

void
fhree(void *ptr)
{
    header_t *hptr;     // header of ptr
    node_t   *prev;     // previous free-node
    node_t   *new_node; // new node to add

    if (ptr == NULL)
        return;

    hptr = ptr - sizeof(header_t);

    // check if hptr is valid
    if (hptr->magic != MAGIC)
        return;

    // unset header as valid
    hptr->magic = 0;

    // new node to add
    new_node       = (node_t *) hptr;
    new_node->size = hptr->size + sizeof(header_t) - sizeof(node_t);

    // get prev node
    prev = head;
    while (prev->next < new_node && prev->next != NULL)
        prev = prev->next;

    // insert new node into list
    new_node->next = prev->next;
    prev->next     = new_node;

    // join with next node if also free
    if ((void *) new_node + sizeof(node_t) + new_node->size == new_node->next)
    {
        new_node->size += new_node->next->size + sizeof(node_t);
        new_node->next = new_node->next->next;
    }

    // join with previous node if also free
    if ((void *) prev + sizeof(node_t) + prev->size == (void *) new_node)
    {
        prev->size += new_node->size + sizeof(node_t);
        prev->next = new_node->next;
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
        return NULL;

    // check if size is valid
    if (size <= 0)
        return NULL;

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
    void     *next        = ptr + hptr->size;
    header_t *hnext       = next;
    node_t   *nnext       = next;
    int       needed_size = size - hptr->size;
    node_t   *prev;

    if (hnext->magic == MAGIC) // next block is used
        return NULL;

    // create next node
    if (nnext->size > needed_size)
    {
        *(node_t *) (next + needed_size) = (node_t){
            .size = nnext->size - needed_size,
            .next = nnext->next,
        };

        hptr->size += needed_size;

        // get prev node
        prev = head;
        while ((void *) prev->next < ptr && prev->next != NULL)
            prev = prev->next;
        prev->next = next + needed_size;

        return ptr;
    }
    else
        return NULL;
}

void *
rehalloc_before(void *ptr, int size, header_t *hptr)
{
    node_t *prev;

    // get prev node
    prev = head;
    while ((void *) prev->next < ptr && prev->next != NULL)
        prev = prev->next;

    // check if previous free block is just before current block
    if ((void *) prev + prev->size == hptr)
    {
        // TODO
    }

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


    // default rehalloc
    // allocate new block and move data, then free current block
    if ((newptr = mhalloc(size)) == NULL) // check for out-of-memmory error
        return NULL;

    blockcpy(newptr, ptr, hptr->size); // move data
    fhree(ptr);

    return newptr;
}

void
print_mem_map(void **ptrs)
{
    node_t   *temp = head;
    header_t *temp2;
    char      l;
    puts("+---------------------+");
    do
    {
        printf("|size: %14d | %p [free]\n", temp->size, temp);
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
            l = ' ';
            for (int i = 0; i < 'z' - 'a' + 1; i++)
                if (ptrs[i] == (void *) temp2 + sizeof(header_t))
                {
                    l = 'a' + i;
                }
            if (temp2->magic == MAGIC)
            {
                printf("|size: %14d | %p [%c]\n", temp2->size, temp2, l);
                printf("|magic: %13x |\n", temp2->magic);
                puts("+---------------------+");
                puts("|used                 |");
                puts("+---------------------+");
                temp2 = (void *) temp2 + temp2->size + sizeof(header_t);
            }
            else
            {
                printf("|size: %14d | %p [free]\n", ((node_t *) temp2)->size, temp2);
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

#include "hallocator.h"
#include <stdio.h>
#include <sys/mman.h>

// global reference to free list (linked list started at head)
node_t *head = NULL;

__attribute__((constructor)) void // call function before main
halloc_init()
{
    // avoid multiple initializations
    if (head == NULL)
    {
        // mmap() return a chunk of free space
        head =
        mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

        head->size = SIZE - sizeof(node_t);
        head->next = NULL;
    }
}


void
fhree(void *ptr)
{
    header_t *hptr = (header_t *) ptr - 1;
    if (hptr->size <= 0 || hptr->magic != MAGIC)
    {
        puts("\e[31mfhree: invalid pointer\e[0m");
        return;
    }
    // unset header pointer as valid chunk pointer
    hptr->magic = 0;

    // get next
    node_t *next; // pointer to next free chunk header
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
        puts("Free chunk just after current");
        *((node_t *) hptr) = (node_t){
            .size = hptr->size + sizeof(header_t) + next->size,
            .next = next->next,
        };
    }
    // check if previous free chunk is just before new one
    else if ((void *) temp + temp->size + sizeof(node_t) == hptr)
    {
        puts("Free chunk just before current");
        temp->size += hptr->size + sizeof(header_t);
        temp->next = next;
    }
    // current chunk is not close to another free chunk
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

    if (head == NULL)
    {
        puts("\e[31hallocator: not initialized correctly\e[0m");
        return ptr;
    }

    // check if size is valid
    if (size <= 0)
    {
        puts("\e[31mmhalloc: invalid size\e[0m");
        return ptr;
    }

    // if size is smaller than headers diff new header dont have enought space at free
    if (size < sizeof(node_t) - sizeof(header_t))
    {
        size = sizeof(node_t) - sizeof(header_t);
    }

    // best fit algorithm (smallest free space greater or equal than size)
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

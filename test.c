#include <stdio.h>
#include <unistd.h>

#define VERBOSE // allow print output from hallocator
#include "hallocator.h"


int
main(int argc, char *argv[])
{
    void *ptrs['z' - 'a' + 1] = { 0 };
    char  action;
    char  c;
    int   n;
    void *r;

    // default
    ptrs[0] = mhalloc(100); // a
    ptrs[1] = mhalloc(100); // b

    while (1)
    {
        puts("\e[H\e[2J");

        print_mem_map(ptrs);

        for (int i = 0; i < 'z' - 'a' + 1; i++)
            if (ptrs[i] != NULL)
                printf("[ptr] %c = %p\n", 'a' + i, ptrs[i]);

        puts("[m]: Mhalloc  id[a-z] size[0,...]    | [q]: Quit");
        puts("[r]: Rehalloc id[a-z] size[0,...]    | [c]: Clear");
        puts("[f]: Fhree    id[a-z]                |");
        printf("[>]: ");
        scanf(" %c", &action);
        switch (action)
        {
            case 'm':
                scanf(" %c %d", &c, &n);
                ptrs[c - 'a'] = mhalloc(n);
                break;

            case 'r':
                scanf(" %c %d", &c, &n);
                ptrs[c - 'a'] = (r = rehalloc(ptrs[c - 'a'], n)) ? r : ptrs[c - 'a'];
                break;

            case 'f':
                scanf(" %c", &c);
                fhree(ptrs[c - 'a']);
                ptrs[c - 'a'] = NULL;
                break;

            case 'c':
                for (int i = 0; i < 'z' - 'a' + 1; i++)
                    if (ptrs[i] != NULL)
                    {
                        fhree(ptrs[i]);
                        ptrs[i] = NULL;
                    }
                break;

            case 'q':
                return 0;

            default:
                fseek(stdin, 0, SEEK_END);
                break;
        }
    }

    return 0;
}

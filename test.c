#include <stdio.h>
#include <unistd.h>

#define VERBOSE // allow print output from hallocator
#include "hallocator.h"

#define s1 10
#define s2 30

int
main(int argc, char *argv[])
{
    int  i;
    int *a, *b;

    mhalloc(10 * sizeof(int));
    mhalloc(10 * sizeof(int));
    a = mhalloc(s1 * sizeof(int));
    b = mhalloc(50 * sizeof(int));
    mhalloc(10 * sizeof(int));
    mhalloc(10 * sizeof(int));
    mhalloc(10 * sizeof(int));
    mhalloc(10 * sizeof(int));

    for (i = 0; i < s1; i++)
        a[i] = i;

    fhree(b);
    a = rehalloc(a, s2 * sizeof(int));

    for (i = s1; i < s2; i++)
        a[i] = i;

    for (i = 0; i < s2; i++)
        printf("%d, ", a[i]);
    putchar('\n');

    return 0;
}

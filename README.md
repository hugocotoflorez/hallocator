# hallocator

Another memory allocator.

### About

This is just a personal project to learn about
memory management and os-related stuff. It is not
efiicient but simple, as Im not trying to improve
existing ones (neither Im able to do it).

### Usage

- `mhalloc(size) -> ptr`: Alloc size bytes and return a pointer to the first byte, or
NULL if it cant allocate that ammount of bytes.
- `fhree(ptr)`: Free an allocated block of memory (allocated using mhalloc).
- `rehalloc(ptr, size) -> nptr`: Alloc size bytes, copy data from ptr to new region
and return a pointer to the new region. ptr is free, so cant be used after calling rehalloc.
If cant allocate more memory, ptr is not free and rehalloc returns NULL.


As this proyect is made for educational reasons, maximum memory that can be allocated is 4Kib.
(Really a little less, due to headers and free-list nodes).

### Contributions

Not allowed, but free to copy the whole code and do
whatever you want with it. For this reason, dont open
any issue or place any comment of any type.

### Sources

Main knowledge source is [ostep](https://pages.cs.wisc.edu/~remzi/OSTEP/#book-chapters).



#ifndef _NEWBOS_HEAP_H
#define _NEWBOS_HEAP_H

#include <stddef.h>

void *
kmalloc(
    size_t size
);

void
kfree(
    void *p
);

#endif

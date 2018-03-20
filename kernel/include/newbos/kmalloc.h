#ifndef _NEWBOS_HEAP_H
#define _NEWBOS_HEAP_H

#include <stdint.h>

void *
kmalloc(
    uint32_t size
);

void
kfree(
    void *p
);

#endif

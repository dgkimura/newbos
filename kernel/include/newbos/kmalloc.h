#ifndef _NEWBOS_HEAP_H
#define _NEWBOS_HEAP_H

#include <stdint.h>

#define KMALLOC_START  0x100000
#define KMALLOC_LENGTH 0x100000

void *
kmalloc(
    uint32_t size
);

void
kfree(
    void *p
);

#endif

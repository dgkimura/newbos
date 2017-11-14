#ifndef _NEWBOS_HEAP_H
#define _NEWBOS_HEAP_H

#include <stddef.h>
#include <stdint.h>

#define KHEAP_START         0xC0000000
#define KHEAP_INITIAL_SIZE  0x00100000
#define HEAP_INDEX_SIZE     0x00020000
#define HEAP_MAGIC          0x123890AB
#define HEAP_MIN_SIZE       0x00070000

uint32_t
kmalloc(
    uint32_t size
);

uint32_t
kmalloc_aligned(
    uint32_t size
);

uint32_t
kmalloc_physical(
    uint32_t size,
    uint32_t *physical_address
);

uint32_t
kmalloc_aligned_physical(
    uint32_t size,
    uint32_t *physical_address
);

void kfree(
    void *p
);

/*
 * Create a new heap.
 */
void create_heap(
    uint32_t start,
    uint32_t end,
    uint32_t max,
    uint8_t supervisor,
    uint8_t readonly
);

#endif

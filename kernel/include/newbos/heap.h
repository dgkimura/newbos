#ifndef _NEWBOS_HEAP_H
#define _NEWBOS_HEAP_H

#include <stddef.h>
#include <stdint.h>

#include <newbos/ordered_array.h>

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

/*
 * Size information for a hole/block
 */
typedef struct
{
    /*
     * Magic number, used for error checking and identification.
     */
    uint32_t magic;

    /*
     * 1 if this is a hole. 0 if this is a block.
     */
    uint8_t is_hole;

    /*
     * Size of the block, including the end footer.
     */
    uint32_t size;
} header_t;

typedef struct
{
    /*
     * Magic number, same as in header_t.
     */
    uint32_t magic;

    /*
     * Pointer to the block header.
     */
    header_t * header;
} footer_t;

typedef struct
{
    ordered_array_t index;

    /*
     * The start of our allocated space.
     */
    uint32_t start_address;

    /*
     * The end of our allocated space. May be expanded up to max_address.
     */
    uint32_t end_address;

    /*
     * The maximum address the heap can be expanded to.
     */
    uint32_t max_address;

    /*
     * Should extra pages requested by us be mapped as supervisor-only?
     */
    uint8_t supervisor;

    /*
     * Should extra pages requested by us be mapped as read-only?
     */
    uint8_t readonly;
} heap_t;

/*
 * Create a new heap.
 */
heap_t *create_heap(
    uint32_t start,
    uint32_t end,
    uint32_t max,
    uint8_t supervisor,
    uint8_t readonly
);

/*
 * Allocates a contiguous region of memory 'sizes' in size. If page_aligned==1,
 * it creates that block starting on a page boundary.
 */
void *alloc(
    size_t size,
    uint8_t page_align,
    heap_t *heap
);

/*
 * Releases a block allocated with 'alloc'.
 */
void
free(
    void *p,
    heap_t *heap
);

#endif

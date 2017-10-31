#include <newbos/heap.h>

#include "paging.h"

extern uint32_t endkernel;

extern page_directory_t *kernel_directory;

uint32_t placement_address = (uint32_t)&endkernel;

static uint32_t
kmalloc_internal(
    uint32_t size,
    int align,
    uint32_t *physical_address
);

uint32_t
kmalloc(
    uint32_t size)
{
    return kmalloc_internal(size, 0, 0);
}

uint32_t
kmalloc_aligned(
    uint32_t size)
{
    return kmalloc_internal(size, 1, 0);
}

uint32_t
kmalloc_physical(
    uint32_t size,
    uint32_t *physical_address)
{
    return kmalloc_internal(size, 0, physical_address);
}

uint32_t
kmalloc_aligned_physical(
    uint32_t size,
    uint32_t *physical_address)
{
    return kmalloc_internal(size, 1, physical_address);
}

static uint32_t
kmalloc_internal(
    uint32_t size,
    int align,
    uint32_t *physical_address)
{
    // If the address is not already page-aligned then align it.
    if (1 == align && placement_address & 0xFFFFF000)
    {
        placement_address &= 0xFFFFF000;
        placement_address += PAGE_SIZE;
    }
    if (physical_address)
    {
        *physical_address = placement_address;
    }

    uint32_t tmp = placement_address;
    placement_address += size;
    return tmp;
}

static int32_t
find_smallest_hole(
    uint32_t size,
    uint8_t page_align,
    heap_t *heap)
{
    /*
     * Find the smallest hole that will fit.
     */
    uint32_t index;

    while (index < heap->index.size)
    {
        header_t *header = (header_t *)lookup_ordered_array(index, &heap->index);

        /*
         * If the user has requested the memory be page-aligned
         */
        if (page_align > 0)
        {
            /*
             * Page-align the starting point of this header.
             */
            uint32_t location = (uint32_t)header;
            int32_t offset = 0;

            if (((location + sizeof(header_t)) & 0xFFFFF000) != 0)
            {
                offset = PAGE_SIZE - (location + sizeof(header_t)) % PAGE_SIZE;
            }

            int32_t hole_size = (int32_t)header->size - offset;

            /*
             * Can we fit now?
             */
            if (hole_size >= (int32_t)size)
            {
                break;
            }
        }
        else if (header->size >= size)
        {
            break;
        }

        index += 1;
    }

    if (index == heap->index.size)
    {
        /*
         * We got to the end and couldn't find anything.
         */
        return -1;
    }
    else
    {
        return index;
    }
}

static int8_t
header_t_less_than(
    void *a,
    void *b)
{
    return (((header_t *)a)->size < ((header_t *)b)->size) ? 1 : 0;
}

heap_t *
create_heap(
    uint32_t start,
    uint32_t end,
    uint32_t max,
    uint8_t supervisor,
    uint8_t readonly)
{
    heap_t *heap = (heap_t *)kmalloc(sizeof(heap_t));

    /*
     * Initialize the index
     */
    heap->index = place_ordered_array((void *)start,
                                      HEAP_INDEX_SIZE,
                                      &header_t_less_than);

    /*
     * Shift the start address forward to resemble where we can start putting
     * data.
     */
    start += sizeof(type_t) * HEAP_INDEX_SIZE;

    /*
     * Make sure the start address is page-aligned.
     */
    if ((start & 0xFFFFF000) != 0)
    {
        start &= 0xFFFFF000;
        start += PAGE_SIZE;
    }

    heap->start_address = start;
    heap->end_address = end;
    heap->max_address = max;
    heap->supervisor = supervisor;
    heap->readonly = readonly;

    header_t *hole = (header_t *)start;
    hole->size = end - start;
    hole->magic = HEAP_MAGIC;
    hole->is_hole = 1;
    insert_ordered_array((void *)hole, &heap->index);

    return heap;
}

static void
expand(
    uint32_t new_size,
    heap_t *heap)
{
    /*
     * Get the nearest page boundary.
     */
    if ((new_size & 0xFFFFF000) != 0)
    {
        new_size &= 0xFFFFF000;
        new_size += PAGE_SIZE;
    }

    /*
     * This should always be on a page boundary.
     */
    uint32_t old_size = heap->end_address - heap->start_address;
    uint32_t i = old_size;

    while (i < new_size)
    {
        alloc_frame(get_page(heap->start_address + i, 1, kernel_directory),
                    (heap->supervisor) ? 1 : 0, (heap->readonly) ? 0 : 1);
        i += PAGE_SIZE;
    }

    heap->end_address = heap->start_address + new_size;
}

static uint32_t
contract(
    uint32_t new_size,
    heap_t *heap)
{
    /*
     * Get the nearest following page boundary.
     */
    if (new_size & PAGE_SIZE)
    {
        new_size &= PAGE_SIZE;
        new_size += PAGE_SIZE;
    }

    /*
     * Don't contract too far!
     */
    if (new_size < HEAP_MIN_SIZE)
    {
        new_size = HEAP_MIN_SIZE;
    }

    uint32_t old_size = heap->end_address - heap->start_address;
    uint32_t i = old_size - PAGE_SIZE;

    while (new_size < i)
    {
        free_frame(get_page(heap->start_address + i, 0, kernel_directory));
        i -= PAGE_SIZE;
    }

    heap->end_address = heap->start_address + new_size;
    return new_size;
}

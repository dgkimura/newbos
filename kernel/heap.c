#include <newbos/heap.h>

extern uint32_t endkernel;

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
        placement_address += 0x1000;
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

            if ((location + sizeof(header_t)) & 0xFFFFF000 != 0)
            {
                offset = 0x1000 - (location + sizeof(header_t)) % 0x1000;
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
header_t_less_than(void *a, void *b)
{
    return (((header_t *)a)->size < ((header_t *)b)->size) ? 1 : 0;
}

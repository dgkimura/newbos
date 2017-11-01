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
    uint32_t index = 0;

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

void *
alloc(
    uint32_t size,
    uint8_t page_align,
    heap_t *heap)
{
    /*
     * Make sure we take the size of the header/footer into account.
     */
    uint32_t new_size = size + sizeof(header_t) + sizeof(footer_t);

    /*
     * Find the smallest hole that will fit.
     */
    int32_t iterator = find_smallest_hole(new_size, page_align, heap);

    if (iterator == -1)
    {
        /*
         * We didn't find a suitable hole.
         */
        uint32_t old_length = heap->end_address - heap->start_address;
        uint32_t old_end_address = heap->end_address;

        /*
         * We need to allocate more space.
         */
        expand(old_length + new_size, heap);

        uint32_t new_length = heap->end_address - heap->start_address;

        /*
         * Find the endmost header. (Not endmost in size, but in location).
         */
        iterator = 0;

        /*
         * Variables to hole the index and value of the endmost header found
         * so far.
         */
        uint32_t idx = -1;
        uint32_t value = 0x0;
        while (iterator < heap->index.size)
        {
            uint32_t tmp = (uint32_t)lookup_ordered_array(iterator,
                                                          &heap->index);
            if (tmp > value)
            {
                value = tmp;
                idx = iterator;
            }
            iterator += 1;
        }

        /*
         * If we didn't find any headers, we need to add one.
         */
        if (idx == -1)
        {
            header_t *header = (header_t *)old_end_address;
            header->magic = HEAP_MAGIC;
            header->size = new_length - old_length;
            header->is_hole = 1;

            footer_t *footer = (footer_t *)(old_end_address + header->size -
                                            sizeof(footer_t));
            footer->magic = HEAP_MAGIC;
            footer->header = header;
            insert_ordered_array((void *)header, &heap->index);
        }
        else
        {
            /*
             * The last header needs adjusting.
             */
            header_t *header = lookup_ordered_array(idx, &heap->index);
            header->size += new_length - old_length;

            /*
             * Rewrite the footer.
             */
            footer_t *footer = (footer_t *)((uint32_t)header + header->size -
                                                     sizeof(footer_t));
            footer->header = header;
            footer->magic = HEAP_MAGIC;
        }
        /*
         * We now have enough space. Recurse, and call the function again.
         */
        return alloc(size, page_align, heap);
    }

    header_t *orig_hole_header = (header_t *)lookup_ordered_array(
        iterator, &heap->index);
    uint32_t orig_hole_pos = (uint32_t)orig_hole_header;
    uint32_t orig_hole_size = orig_hole_header->size;

    /*
     * Here we work out oif we should split the hole we found into two parts.
     * Is the original hole size - requested hole size less than the overhead
     * for adding a new hole?
     */
    if (orig_hole_size - new_size < sizeof(header_t) + sizeof(footer_t))
    {
        /*
         * Then just increase the requested size to the sizew of the hole we
         * found.
         */
        size += orig_hole_size -  new_size;
        new_size = orig_hole_size;
    }

    /*
     * If we need to page-align the data, do it now and make a new hole in
     * front of our block.
     */
    if (page_align && orig_hole_pos & 0xFFFFF000)
    {
        uint32_t size = PAGE_SIZE + (orig_hole_pos & 0xFFF) - sizeof(header_t);
        uint32_t new_location = orig_hole_pos + size;
        header_t *hole_header = (header_t *)orig_hole_pos;
        hole_header->size = size;
        hole_header->magic = HEAP_MAGIC;
        hole_header->is_hole = 1;

        footer_t *hole_footer = (footer_t *)(new_location - sizeof(footer_t));
        hole_footer->magic = HEAP_MAGIC;
        hole_footer->header = hole_header;
        orig_hole_pos = new_location;
        orig_hole_size = orig_hole_size - hole_header->size;
    }
    else
    {
        /*
         * Else we don't need this hole any more, delete it from the index.
         */
        remove_ordered_array(iterator, &heap->index);
    }

    /*
     * Overwrite the original header and footer.
     */
    header_t *block_header = (header_t *)orig_hole_header;
    block_header->magic = HEAP_MAGIC;
    block_header->is_hole = 0;
    block_header->size = new_size;
    footer_t *block_footer = (footer_t *)(orig_hole_pos - sizeof(footer_t) +
                                          size);
    block_footer->magic = HEAP_MAGIC;
    block_footer->header = block_header;

    /*
     * We may need to write a new hole after the allocated block. We do this
     * only if the new hole would have positive size.
     */
    if (orig_hole_size - new_size > 0)
    {
        header_t *hole_header = (header_t *)(orig_hole_pos + sizeof(header_t) +
                                             size + sizeof(footer_t));
        hole_header->magic = HEAP_MAGIC;
        hole_header->is_hole = 1;
        hole_header->size = orig_hole_size - new_size;

        footer_t *hole_footer = (footer_t *)((uint32_t)hole_header +
                                                       orig_hole_size -
                                                       new_size -
                                                       sizeof(footer_t));
        if ((uint32_t)hole_footer < heap->end_address)
        {
            hole_footer->magic = HEAP_MAGIC;
            hole_footer->header = hole_header;
        }

        /*
         * Put the new hole in the index.
         */
        insert_ordered_array((void *)hole_header, &heap->index);
    }

    return (void *)((uint32_t)block_header + sizeof(header_t));
}
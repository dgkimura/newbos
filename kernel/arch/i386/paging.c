#include <stdlib.h>
#include <string.h>

#include <newbos/tty.h>
#include <newbos/heap.h>

#include "paging.h"

/*
 * The kernel's page directory
 */
page_directory_t *kernel_directory = 0;

/*
 * The current page directory
 */
page_directory_t *current_directory = 0;

/*
 * Used/free bitmap of frames
 */
uint32_t *frames;
uint32_t nframes;

extern uint32_t placement_address;

extern void
enable_paging(
    uint32_t
);

extern uint32_t
get_fault_address(
    void
);

extern void copy_page_physical(
    uint32_t,
    uint32_t
);

#define INDEX_FROM_BIT(a) (a / (8 * 4))
#define OFFSET_FROM_BIT(a) (a % (8 * 4))

static void
set_frame(
    uint32_t frame_address)
{
    uint32_t frame = frame_address / PAGE_SIZE;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offet = OFFSET_FROM_BIT(frame);
    frames[index] |= (0x1 << offet);
}

static void
clear_frame(
    uint32_t frame_address)
{
    uint32_t frame = frame_address / PAGE_SIZE;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offet = OFFSET_FROM_BIT(frame);
    frames[index] &= ~(0x1 << offet);
}

static uint32_t
test_frame(
    uint32_t frame_address)
{
    uint32_t frame = frame_address / PAGE_SIZE;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offet = OFFSET_FROM_BIT(frame);
    return (frames[index] & (0x1 << offet));
}

static uint32_t
first_frame(
    void)
{
    uint32_t i, j;
    for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
    {
        if (frames[i] != 0xFFFFFFFF)
        {
            for (j = 0; j < 32; j++)
            {
                if (!(frames[i] & (0x1 << j)))
                {
                    return i * 4 * 8 + j;
                }
            }
        }
    }
    return -1;
}

void
alloc_frame(
    page_t *page,
    int is_kernel,
    int is_writable)
{
    if (0 != page->frame)
    {
        /*
         * Frame was already allocated.
         */
        return;
    }
    else
    {
        uint32_t index = first_frame();
        if (((uint32_t)-1) == index)
        {
            monitor_write("No free frames!\n");
        }

        set_frame(index * PAGE_SIZE);
        page->present = 1;
        page->rw = (is_writable) ? 1 : 0;
        page->user = (is_kernel) ? 0 : 1;
        page->frame = index;
    }
}

void
free_frame(
    page_t *page)
{
    uint32_t frame;
    if (!(frame = page->frame))
    {
        /*
         * Page didn't actually have an allocated frame.
         */
        return;
    }
    else
    {
        clear_frame(frame);
        page->frame = 0x0;
    }
}

void
init_paging(
    void)
{
    /*
     * The size of physical memory. For the moment we assume it is 16 MB.
     */
    uint32_t memory_end_page = 0x1000000;

    nframes = memory_end_page / PAGE_SIZE;
    frames = (uint32_t *)kmalloc(INDEX_FROM_BIT(nframes));
    memset(frames, 0, INDEX_FROM_BIT(nframes));

    /*
     * Let's make a page directory.
     */
    kernel_directory = (page_directory_t *)kmalloc_aligned(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physical_address = (uint32_t)kernel_directory->physical_tables;

    /*
     * Map some pages in the kernel heap area. Here we call get_page but not
     * alloc_page. This causes page_table_t's to be created where necessary. We
     * can't allocate frames yet because they need to be identity mapped first
     * below, and yet we can't increase placement_address between identity
     * mapping and enabling the heap!
     */
    for (uint32_t i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE;
         i += PAGE_SIZE)
    {
        get_page(i, 1, kernel_directory);
    }

    /*
     * We need to identify map (physical address = virtual address) from 0x0 to
     * the end of used memory, so we can access this transparently, as if
     * paging wasn't enabled. NOTE that we use a while loop here deliberately.
     * Inside the loop body we actually change placement_address by calling
     * kmalloc(). A while loop causes this to be computed on-the-fly rather
     * than once at start. Allocate an extra PAGE so the kernel heap can be
     * initialized properly.
     */
    for (uint32_t i = 0; i < placement_address + PAGE_SIZE; i += PAGE_SIZE)
    {
        alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
    }

    /*
     * Now allocate those pages we mapped earlier without allocating frames.
     */
    for (uint32_t i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE;
         i += PAGE_SIZE)
    {
        alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
    }

    /*
     * Before we enable paging, we must register our page fault handler.
     */
    register_isr_handler(14, page_fault);

    /*
     * Now, enable paging!
     */
    switch_page_directory(kernel_directory);

    /*
     * Initialize the kernel heap.
     */
    create_heap(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE,
                       0xCFFFF000, 0, 0);

    current_directory = clone_page_directory(kernel_directory);
    switch_page_directory(current_directory);
}

void
switch_page_directory(
    page_directory_t *directory)
{
    current_directory = directory;
    enable_paging((uint32_t)directory->physical_address);
}

page_directory_t *
clone_page_directory(
    page_directory_t *source)
{
    uint32_t physical;

    /*
     * Make a new page directory and obtain its physical address.
     */
    page_directory_t *directory = (page_directory_t *)
        kmalloc_aligned_physical(sizeof(page_directory_t), &physical);
    memset(directory, 0, sizeof(page_directory_t));

    /*
     * Get the offset of physical_tables from the start of the page_directory_t
     * structure.
     */
    uint32_t offset = (uint32_t)directory->physical_tables -
                      (uint32_t)directory;

    /*
     * Then the physical address of directory is...
     */
    directory->physical_address = physical + offset;

    /*
     * Go through each page table. If the page table is in the kernel
     * directory, do not need to make a copy.
     */
    for (int i = 0; i < 1024; i++)
    {
        if (!(source->tables[i]))
        {
            continue;
        }

        if (kernel_directory->tables[i] == source->tables[i])
        {
            /*
             * It's in the kernel, so just use the same pointer.
             */
            directory->tables[i] = source->tables[i];
            directory->physical_tables[i] = source->physical_tables[i];
        }
        else
        {
            /*
             * Copy the table.
             */
            uint32_t physical;
            directory->tables[i] = clone_page_table(source->tables[i],
                                                    &physical);
            directory->physical_tables[i] = physical | 0x07;
        }
    }
    return directory;
}

page_table_t *
clone_page_table(
    page_table_t *source,
    uint32_t *physical_address)
{
    /*
     * Make a new page table, which is page aligned.
     */
    page_table_t *table = (page_table_t *)kmalloc_aligned_physical(
        sizeof(page_table_t), physical_address);

    memset(table, 0, sizeof(page_table_t));

    for (int i = 0; i < 1024; i++)
    {
        if (source->pages[i].frame)
        {
            /*
             * Get a new frame.
             */
            alloc_frame(&table->pages[i], 0, 0);
            if (source->pages[i].present)
            {
                table->pages[i].present = 1;
            }
            if (source->pages[i].rw)
            {
                table->pages[i].rw = 1;
            }
            if (source->pages[i].user)
            {
                table->pages[i].user = 1;
            }
            if (source->pages[i].accessed)
            {
                table->pages[i].accessed = 1;
            }
            if (source->pages[i].dirty)
            {
                table->pages[i].dirty = 1;
            }

            /*
             * Physically copy the data across.
             */
            copy_page_physical(source->pages[i].frame * PAGE_SIZE,
                               table->pages[i].frame * PAGE_SIZE);
        }
    }

    return table;
}

page_t *
get_page(
    uint32_t address,
    int make,
    page_directory_t *directory)
{
    /*
     * Turn the address into an index.
     */
    address /= PAGE_SIZE;

    /*
     * Find the page table containing this address.
     */
    uint32_t table_index = address / 1024;

    if (directory->tables[table_index])
    {
        return &directory->tables[table_index]->pages[address % 1024];
    }
    else if (make)
    {
        uint32_t tmp;
        directory->tables[table_index] = (page_table_t *)kmalloc_aligned_physical(
            sizeof(page_directory_t), &tmp);
        memset(directory->tables[table_index], 0, PAGE_SIZE);
        directory->physical_tables[table_index] = tmp | 0x7;
        return &directory->tables[table_index]->pages[address % 1024];
    }
    else
    {
        return 0;
    }
}

void
page_fault(
    registers_t *regs)
{
    /*
     * A page fault has occurred.
     * The faulting address is stored in the CR2 register.
     */
    uint32_t fault_address = get_fault_address();

    int present = !(regs->error_code & 0x1);
    int rw = regs->error_code & 0x2;
    int user = regs->error_code & 0x4;
    int reserved = regs->error_code & 0x8;

    monitor_write("Page fault! (");
    if (present)
    {
        monitor_write("present ");
    }
    if (rw)
    {
        monitor_write("read-only ");
    }
    if (user)
    {
        monitor_write("user-mode ");
    }
    if (reserved)
    {
        monitor_write("reserved ");
    }
    monitor_write(") at 0x ");
    monitor_write_hex(fault_address);
    monitor_write("\n");
    abort();
}

#include <stdlib.h>
#include <string.h>
#include <newbos/tty.h>

#include "mm.h"
#include "paging.h"

// The kernel's page directory
page_directory_t *kernel_directory = 0;

// The current page directory
page_directory_t *current_directory = 0;

// Used/free bitmap of frames
uint32_t *frames;
uint32_t nframes;

extern uint32_t placement_address;

extern void enable_paging(uint32_t);

extern uint32_t get_fault_address();

#define INDEX_FROM_BIT(a) (a / (8 * 4))
#define OFFSET_FROM_BIT(a) (a % (8 * 4))

static void
set_frame(uint32_t frame_address)
{
    uint32_t frame = frame_address / PAGE_SIZE;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offet = OFFSET_FROM_BIT(frame);
    frames[index] |= (0x1 << offet);
}

static void
clear_frame(uint32_t frame_address)
{
    uint32_t frame = frame_address / PAGE_SIZE;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offet = OFFSET_FROM_BIT(frame);
    frames[index] &= ~(0x1 << offet);
}

static uint32_t
test_frame(uint32_t frame_address)
{
    uint32_t frame = frame_address / PAGE_SIZE;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offet = OFFSET_FROM_BIT(frame);
    return (frames[index] & (0x1 << offet));
}

static uint32_t
first_frame()
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
alloc_frame(page_t *page, int is_kernel, int is_writable)
{
    if (0 != page->frame)
    {
        // Frame was already allocated.
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
free_frame(page_t *page)
{
    uint32_t frame;
    if (!(frame = page->frame))
    {
        // Page didn't actually have an allocated frame.
        return;
    }
    else
    {
        clear_frame(frame);
        page->frame = 0x0;
    }
}

void
init_paging()
{
    // The size of physical memory. For the moment we assume it is 16 MB.
    uint32_t memory_end_page = 0x1000000;

    nframes = memory_end_page / PAGE_SIZE;
    frames = (uint32_t *)kmalloc(INDEX_FROM_BIT(nframes));
    memset(frames, 0, INDEX_FROM_BIT(nframes));

    // Let's make a page directory.
    kernel_directory = (page_directory_t *)kmalloc_aligned(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));
    current_directory = kernel_directory;

    // We need to identify map (physical address = virtual address) from 0x0 to
    // the end of used memory, so we can access this transparently, as if
    // paging wasn't enabled. NOTE that we use a while loop here deliberately.
    // Inside the loop body we actually change placement_address by calling
    // kmalloc(). A while loop causes this to be computed on-the-fly rather
    // than once at start.
    uint32_t i = 0;
    while (i < placement_address)
    {
        alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
        i += PAGE_SIZE;
    }

    // Before we enable paging, we must register our page fault handler.
    register_isr_handler(14, page_fault);

    // Now, enable paging!
    switch_page_directory(kernel_directory);
}

void
switch_page_directory(page_directory_t *directory)
{
    current_directory = directory;
    enable_paging((uint32_t)&directory->physical_tables);
}

page_t *
get_page(uint32_t address, int make, page_directory_t *directory)
{
    // Turn the address into an index.
    address /= PAGE_SIZE;

    // Find the page table containing this address.
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

void page_fault(registers_t *regs)
{
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
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

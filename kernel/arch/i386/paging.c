#include <stdint.h>
#include <string.h>

#include <newbos/paging.h>

#include "memory.h"

static uint32_t *freemap;
static uint32_t freemap_frames;

#define FRAME_SIZE (8*sizeof(*freemap))
#define PAGE_ALIGNMENT 12

void
frames_init(
    void)
{
    uint32_t i;
    uint32_t freemap_bits = MAIN_MEMORY_SIZE/PAGE_SIZE;
    uint32_t freemap_bytes = freemap_bits / 8;
    uint32_t freemap_pages = 1 + freemap_bytes / PAGE_SIZE;

    freemap = (void *)MAIN_MEMORY_START;

    freemap_frames = 1 + freemap_bits / FRAME_SIZE;

    memset(freemap, 0xFF, freemap_bytes);
    for (i=0; i<freemap_pages; i++)
    {
        frame_allocate();
    }
}

void *
frame_allocate(
    void)
{
    uint32_t i, j, frame_mask, page_number;
    void *address = 0;

    for (i=1; i<freemap_frames; i++)
    {
        if (freemap[i] != 0)
        {
            for (j=0; j<FRAME_SIZE; j++)
            {
                frame_mask = (1<<j);
                if (freemap[i] & frame_mask)
                {
                    freemap[i] &= ~frame_mask;
                    page_number = i * FRAME_SIZE + j;
                    address = (page_number << PAGE_ALIGNMENT) +
                              (void *)MAIN_MEMORY_START;
                }
            }
        }
    }
    return address;
}

void
frame_free(
    void *address)
{
    uint32_t page_number = ((int32_t)address - MAIN_MEMORY_START) >> PAGE_ALIGNMENT;
    uint32_t frame_number = page_number / FRAME_SIZE;
    uint32_t frame_offset = page_number % FRAME_SIZE;
    uint32_t frame_mask = (1<<frame_offset);
    freemap[frame_number] |= frame_mask;
}

struct pagetable *
pagetable_create(
    void)
{
    struct pagetable *frame = frame_allocate();
    memset((void *)frame, 0, PAGE_SIZE);
    return frame;
}

void
pagetable_init(
    struct pagetable *p)
{
    unsigned int i;

    for (i=0; i<MAIN_MEMORY_SIZE; i+=PAGE_SIZE)
    {
        pagetable_map(p, i, i, PAGE_FLAG_KERNEL|PAGE_FLAG_READWRITE);
    }
}

void
pagetable_alloc(
    struct pagetable *p,
    uint32_t virtual_address,
    uint32_t length,
    uint32_t flags)
{
    uint32_t npages = length / PAGE_SIZE;

    if(length % PAGE_SIZE)
    {
        npages += 1;
    }

    virtual_address &= 0xfffff000;

    while (npages > 0)
    {
        uint32_t physical_address;
        if(!pagetable_get(p, virtual_address, &physical_address))
        {
            pagetable_map(p, virtual_address, 0, flags|PAGE_FLAG_ALLOC);
        }
        virtual_address += PAGE_SIZE;
        npages -= 1;
    }
}

void
pagetable_map(
    struct pagetable *p,
    uint32_t virtual_address,
    uint32_t physical_address,
    uint32_t flags)
{
    struct pagetable *q;
    struct pagetable_entry *e;

    if (flags & PAGE_FLAG_ALLOC)
    {
        physical_address = (uint32_t)frame_allocate();

        if(!physical_address)
        {
            return;
        }
    }

    e = &p->entry[virtual_address>>22];

    if (!e->present)
    {
        q = (struct pagetable *)frame_allocate();
        if (!q)
        {
            return;
        }
        memset((void *)q, 0, PAGE_SIZE);

        e->present = 1;
        e->readwrite = 1;
        e->user = (flags & PAGE_FLAG_KERNEL) ? 0 : 1;
        e->writethrough = 0;
        e->nocache = 0;
        e->accessed = 0;
        e->dirty = 0;
        e->pagesize = 0;
        e->globalpage = (flags & PAGE_FLAG_KERNEL) ? 1 : 0;
        e->available = 0;
        e->address = (((uint32_t)q) >> 12);
    }
    else
    {
        q = (struct pagetable *)(((uint32_t)e->address) << 12);
    }

    e = &q->entry[(virtual_address>>12) & 0x3FF];

    e->present = 1;
    e->readwrite = (flags & PAGE_FLAG_READWRITE) ? 1 : 0;
    e->user = (flags & PAGE_FLAG_KERNEL) ? 0 : 1;
    e->writethrough = 0;
    e->nocache = 0;
    e->accessed = 0;
    e->dirty = 0;
    e->pagesize = 0;
    e->globalpage = !e->user;
    e->available = (flags & PAGE_FLAG_ALLOC) ? 1 : 0;
    e->address = (physical_address >> 12);
}

int
pagetable_get(
    struct pagetable *p,
    uint32_t virtual_address,
    uint32_t *physical_address)
{
    struct pagetable *q;
    struct pagetable_entry *e;

    e = &p->entry[virtual_address >> 22];
    if(!e->present)
    {
        return 0;
    }

    q = (struct pagetable*) (e->address << 12);

    e = &q->entry[(virtual_address>>12) & 0x3FF];
    if(!e->present)
    {
        return 0;
    }

    *physical_address = e->address << 12;
    return 1;
}

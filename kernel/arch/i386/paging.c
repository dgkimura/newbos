#include <newbos/tty.h>

#include "paging.h"

// Used/free bitmap of frames
uint32_t *frames;
uint32_t nframes;

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

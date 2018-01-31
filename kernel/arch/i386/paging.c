#include <stdint.h>
#include <string.h>

#include <newbos/paging.h>

#include "memory.h"

static uint32_t *freemap;
static uint32_t freemap_frames;

#define FRAME_SIZE (8*sizeof(*freemap))
#define PAGE_SIZE 0x1000
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

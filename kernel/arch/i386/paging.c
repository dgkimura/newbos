#include "paging.h"

// Used or free bitmap of frames
uint32_t *frames;
uint32_t nframes;

#define INDEX_FROM_BIT(a) (a / (8 * 4))
#define OFFSET_FROM_BIT(a) (a % (8 * 4))

static void
set_frame(uint32_t frame_address)
{
    uint32_t frame = frame_address / 0x1000;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offet = OFFSET_FROM_BIT(frame);
    frames[index] |= (0x1 << offet);
}

static void
clear_frame(uint32_t frame_address)
{
    uint32_t frame = frame_address / 0x1000;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offet = OFFSET_FROM_BIT(frame);
    frames[index] &= ~(0x1 << offet);
}

static uint32_t
test_frame(uint32_t frame_address)
{
    uint32_t frame = frame_address / 0x1000;
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

#include "mm.h"

extern uint32_t endkernel;
uint32_t placement_address = (uint32_t)&endkernel;

uint32_t
kmalloc_aligned(uint32_t size)
{
    return kmalloc_internal(size, 1, 0);
}

uint32_t
kmalloc_physical(uint32_t size, uint32_t *physical_address)
{
    return kmalloc_internal(size, 0, physical_address);
}

uint32_t
kmalloc_aligned_physical(uint32_t size, uint32_t *physical_address)
{
    return kmalloc_internal(size, 1, physical_address);
}

uint32_t
kmalloc_internal(uint32_t size, int align, uint32_t *physical_address)
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

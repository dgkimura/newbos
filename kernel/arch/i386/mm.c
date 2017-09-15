#include "mm.h"

extern uint32_t endkernel;
uint32_t placement_address = (uint32_t)&endkernel;

uint32_t
kalloc(uint32_t size, int align, uint32_t* physical_address)
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

#ifndef _NEWBOS_MM_H
#define _NEWBOS_MM_H

#include <stdint.h>

uint32_t kmalloc_aligned(uint32_t size);

uint32_t kmalloc_physical(uint32_t size, uint32_t *physical_address);

uint32_t kmalloc_aligned_physical(uint32_t size, uint32_t *physical_address);

static uint32_t kmalloc_internal(uint32_t size, int align,
                                 uint32_t *physical_address);

#endif

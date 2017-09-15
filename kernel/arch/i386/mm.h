#ifndef _NEWBOS_MM_H
#define _NEWBOS_MM_H

#include <stdint.h>

uint32_t kalloc(uint32_t size, int align, uint32_t *physical_address);

#endif

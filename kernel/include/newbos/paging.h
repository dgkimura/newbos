#ifndef _NEWBOS_PAGING_H
#define _NEWBOS_PAGING_H

#include <stdint.h>

#include "multiboot.h"

#define PAGE_SIZE 0x1000

#define PAGING_READ_ONLY  0
#define PAGING_READ_WRITE 1
#define PAGING_PL0        0
#define PAGING_PL3        1

void
frames_init(
    uint32_t kernel_physical_start,
    uint32_t kernel_physical_end,
    uint32_t kernel_virtual_start,
    uint32_t kernel_virtual_end,
    uint32_t kernel_pdt_vaddr,
    uint32_t kernel_pt_vaddr,
    struct multiboot_info *minfo
);

uint32_t
pfa_allocate(
    uint32_t num_page_frames
);

uint32_t
pdt_map_kernel_memory(
    uint32_t paddr,
    uint32_t vaddr,
    uint32_t size,
    uint8_t rw,
    uint8_t pl
);

#endif

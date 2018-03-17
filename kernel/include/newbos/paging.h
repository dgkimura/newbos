#ifndef _NEWBOS_PAGING_H
#define _NEWBOS_PAGING_H

#include <stdint.h>

#include "multiboot.h"

#define PAGE_SIZE 0x1000

#define PAGE_FLAG_USER        0
#define PAGE_FLAG_KERNEL      1
#define PAGE_FLAG_EXISTS      0
#define PAGE_FLAG_ALLOC       2
#define PAGE_FLAG_READONLY    0
#define PAGE_FLAG_READWRITE   4
#define PAGE_FLAG_NOCLEAR     0
#define PAGE_FLAG_CLEAR       8

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

#endif

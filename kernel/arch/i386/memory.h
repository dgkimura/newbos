#ifndef _NEWBOS_MEMORY_H
#define _NEWBOS_MEMORY_H

#define MAIN_MEMORY_START 0x200000

#define MAIN_MEMORY_SIZE  0x8000000

#define KERNEL_START_VADDR  0xC0000000

#define KERNEL_PDT_IDX (KERNEL_START_VADDR >> 22)

#define PHYSICAL_TO_VIRTUAL(addr) ((addr) + KERNEL_START_VADDR)

#define VIRTUAL_TO_PHYSICAL(addr) ((addr) - KERNEL_START_VADDR)

#endif

#ifndef _NEWBOS_GDT_H
#define _NEWBOS_GDT_H

#include <stdint.h>

void
gdt_init(
    uint32_t tss_vaddr
);

#endif

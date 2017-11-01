#ifndef _NEWBOS_PAGING_H
#define _NEWBOS_PAGING_H

#include <stdint.h>

#include "interrupts.h"

#define PAGE_SIZE 0x1000

typedef struct page
{
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20;
} page_t;

typedef struct page_table
{
    page_t pages[1024];
} page_table_t;

typedef struct page_directory
{
    // Array of poitners to page tables.
    page_table_t *tables[1024];

    // Array of pointers to page_tables, but gives their physical location for
    // loading into the CR3 register.
    uint32_t physical_tables[1024];

    // The physical address of tables_physical. This comes into play when we
    // get our kernel heap allocated and the directory may be in a different
    // location in virtual memory.
    uint32_t physical_address;
} page_directory_t;

void
init_paging(
    void
);

void
switch_page_directory(
    page_directory_t *directory
);

page_t *
get_page(
    uint32_t address,
    int make,
    page_directory_t *directory
);

void
page_fault(
    registers_t *regs
);

void
alloc_frame(
    page_t *page,
    int is_kernel,
    int is_writable
);

void
free_frame(
    page_t *page
);

#endif

#ifndef _NEWBOS_PROCESS_H
#define _NEWBOS_PROCESS_H

#include <stdint.h>

#include <newbos/paging.h>

#define PROCESS_ENTRY_POINT 0x80000000
#define PROCESS_STACK_INIT  0xfffffff0

struct process
{
    struct pagetable *pagetable;
    void *kernel_stack;
    void *process_stack;
    uint32_t entry;
};

void
process_init(
    void
);

struct process *
process_create(
    uint32_t code_size,
    uint32_t stack_size
);

#endif

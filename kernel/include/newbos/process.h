#ifndef _NEWBOS_PROCESS_H
#define _NEWBOS_PROCESS_H

#include <stdint.h>

#include <newbos/paging.h>

struct _registers {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t ss;
    uint32_t esp;
    uint32_t eflags;
    uint32_t cs;
    uint32_t eip;
} __attribute__((packed));

struct paddr_ele {
    uint32_t paddr;
    uint32_t count;
    struct paddr_ele *next;
};

struct paddr_list {
    struct paddr_ele *start;
    struct paddr_ele *end;
};

struct process
{
    uint32_t id;
    uint32_t parent_id;

    struct pde *pdt;
    uint32_t pdt_paddr;

    struct _registers current;
    struct _registers user_mode;

    uint32_t kernel_stack_start_vaddr;
    uint32_t stack_start_vaddr;
    uint32_t code_start_vaddr;

    struct paddr_list code_paddrs;
    struct paddr_list stack_paddrs;
    struct paddr_list kernel_stack_paddrs;
};

void
process_init(
    void
);

struct process *
process_create(
    char const *path,
    uint32_t id
);

#endif

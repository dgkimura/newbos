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

struct tss {
    uint16_t prev_task_link;
    uint16_t reserved;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved0;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved1;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved2;

    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint16_t es;
    uint16_t reserved3;
    uint16_t cs;
    uint16_t reserved4;
    uint16_t ss;
    uint16_t reserved5;
    uint16_t ds;
    uint16_t reserved6;
    uint16_t fs;
    uint16_t reserved7;
    uint16_t gs;
    uint16_t reserved8;

    uint16_t ldt_ss;
    uint16_t reserved9;

    uint16_t debug_and_reserved; /* The lowest bit is for debug */
    uint16_t io_map_base;

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

uint32_t
tss_init(
    void
);

void
tss_set_kernel_stack(
    uint16_t segsel,
    uint32_t vaddr
);

void
process_init(
    void
);

struct process *
process_create(
    char const *path
);

#endif

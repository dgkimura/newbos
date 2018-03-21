#include <string.h>

#include <newbos/kmalloc.h>
#include <newbos/process.h>
#include <newbos/printk.h>

#include "memory.h"

#define FOUR_KB     0x1000
#define PROC_INITIAL_STACK_SIZE 1 /* in page frames */
#define PROC_INITIAL_STACK_VADDR (KERNEL_START_VADDR - FOUR_KB)
#define PROC_INITIAL_ESP (KERNEL_START_VADDR - 4)

#define KERNEL_STACK_SIZE FOUR_KB

/*
 * segements
 */
#define SEGSEL_USER_SPACE_CS 0x18
#define SEGSEL_USER_SPACE_DS 0x20

/*
 * registers
 */
#define REG_EFLAGS_DEFAULT 0x202

static uint32_t
div_ceil( uint32_t num, uint32_t den);

void
process_init(
    void)
{
    process_create("/bin/init", 1);
}

struct process *
process_create(
    char const *path,
    uint32_t id)
{
    struct process *p;

    p = (struct process *)kmalloc(sizeof(struct process));
    if (NULL == p)
    {
        printk("Failed to kmalloc 'struct process' during process create.");
    }

    /*
     * Initialize process structure.
     */
    p->id = id;
    p->parent_id = 0;
    p->pdt = 0;
    p->pdt_paddr = 0;
    p->kernel_stack_start_vaddr = 0;
    p->code_start_vaddr = 0;
    p->stack_start_vaddr = PROC_INITIAL_STACK_VADDR;
    p->code_paddrs.start = NULL;
    p->code_paddrs.end = NULL;
    p->stack_paddrs.start = NULL;
    p->stack_paddrs.end = NULL;
    p->kernel_stack_paddrs.start = NULL;
    p->kernel_stack_paddrs.end = NULL;

    memset(&p->user_mode, 0, sizeof(struct _registers));
    memset(&p->current, 0, sizeof(struct _registers));

    p->user_mode.eflags = REG_EFLAGS_DEFAULT;
    p->user_mode.ss = (SEGSEL_USER_SPACE_DS | 0x03);
    p->user_mode.cs = (SEGSEL_USER_SPACE_CS | 0x03);

    /*
     * Load process page tables
     */
    {
        uint32_t paddr;
        struct pdt *pdt = pdt_create(&paddr);
        if (pdt == NULL || paddr == 0)
        {
            printk("process_load_pdt: Could not create PDT for process."
                   "pdt: %X, pdt_paddr: %u\n",
                      (uint32_t) pdt, paddr);
            return NULL;
        }
        p->pdt = pdt;
        p->pdt_paddr = paddr;
    }

    /*
     * TODO: Load process code
     */

    /*
     * Load process stack
     */
    {
        uint32_t paddr, bytes, pfs, mapped_memory_size;
        struct paddr_ele *stack_paddrs;
        pfs = div_ceil(PROC_INITIAL_STACK_SIZE, FOUR_KB);
        paddr = pfa_allocate(pfs);
        if (paddr == 0)
        {
            printk("process_load_stack: Could not allocate page frames for "
                   "stack. pfs: %u\n", pfs);
            return NULL;
        }

        bytes = pfs * FOUR_KB;
        mapped_memory_size = pdt_map_memory(p->pdt, paddr,
            PROC_INITIAL_STACK_VADDR, bytes, PAGING_READ_WRITE, PAGING_PL3);
        if (mapped_memory_size < bytes)
        {
            printk("process_load_stack: Could not map memory for stack in "
                   "given pdt. vaddr: %X, paddr: %X, size: %u, pdt: %X\n",
                   PROC_INITIAL_STACK_VADDR, paddr, bytes, (uint32_t) p->pdt);
            return NULL;
        }

        stack_paddrs = kmalloc(sizeof(struct paddr_ele));
        if (stack_paddrs == NULL)
        {
            printk("process_load_stack: Could not allocated memory for stack "
                   "paddr list\n");
            return NULL;
        }

        stack_paddrs->paddr = paddr;
        stack_paddrs->count = pfs;
        stack_paddrs->next = NULL;

        p->stack_paddrs.start = stack_paddrs;
        p->stack_paddrs.end = stack_paddrs;
        p->stack_start_vaddr = PROC_INITIAL_STACK_VADDR;
        p->user_mode.esp = PROC_INITIAL_ESP;
    }

    /*
     * Load process kernel stack
     */
    {
        uint32_t pfs, bytes, vaddr, paddr, mapped_memory_size;
        struct paddr_ele *kernel_stack_paddrs;

        pfs = div_ceil(KERNEL_STACK_SIZE, FOUR_KB);
        paddr = pfa_allocate(pfs);
        if (paddr == 0) {
            printk("process_load_kernel_stack: Could not allocate page for "
                   "kernel stack. pfs: %u\n", pfs);
            return NULL;
        }

        bytes = pfs * FOUR_KB;
        vaddr = pdt_kernel_find_next_vaddr(bytes);
        if (vaddr == 0) {
            printk("process_load_kernel_stack: Could not find virtual address "
                   "for kernel stack."
                   "bytes: %u\n", bytes);
            return NULL;
        }

        mapped_memory_size =
            pdt_map_kernel_memory(paddr, vaddr, bytes, PAGING_READ_WRITE,
                                  PAGING_PL0);
        if (mapped_memory_size != bytes) {
            printk("process_load_kernel_stack: Could not map memory for "
                   "kernel stack. paddr: %X, vaddr: %X, bytes: %u\n",
                    paddr, vaddr, bytes);
            return NULL;
        }

        kernel_stack_paddrs = kmalloc(sizeof(struct paddr_ele));
        if (kernel_stack_paddrs == NULL) {
            printk("process_load_kernel_stack: Could not allocated memory for "
                   "kernel stack paddr list\n");
            return NULL;
        }

        kernel_stack_paddrs->count = pfs;
        kernel_stack_paddrs->paddr = paddr;
        kernel_stack_paddrs->next = NULL;

        p->kernel_stack_paddrs.start = kernel_stack_paddrs;
        p->kernel_stack_paddrs.end = kernel_stack_paddrs;
        p->kernel_stack_start_vaddr = vaddr + bytes - 4;
    }

    return p;
}

static uint32_t
div_ceil(
    uint32_t num,
    uint32_t den)
{
    return (num - 1) / den + 1;
}

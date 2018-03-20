#include <string.h>

#include <newbos/kmalloc.h>
#include <newbos/process.h>
#include <newbos/printk.h>

#include "memory.h"

#define FOUR_KB     0x1000
#define PROC_INITIAL_STACK_VADDR (KERNEL_START_VADDR - FOUR_KB)

/*
 * segements
 */
#define SEGSEL_USER_SPACE_CS 0x18
#define SEGSEL_USER_SPACE_DS 0x20

/*
 * registers
 */
#define REG_EFLAGS_DEFAULT 0x202

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

    /*
     * TODO: Load process code
     */

    /*
     * TODO: Load process stack
     */

    /*
     * TODO: Load process kernel stack
     */

    return p;
}

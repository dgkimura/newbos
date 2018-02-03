#include <string.h>

#include <newbos/process.h>

struct process *current;

void
process_init(
    void)
{
    current = process_create(0, 0);

    pagetable_load(current->pagetable);
    pagetable_enable();
}

struct process *
process_create(
    uint32_t code_size,
    uint32_t stack_size)
{
    struct process *p;
    p = (struct process *)frame_allocate();
    memset(p, 0, PAGE_SIZE);

    p->pagetable = (struct pagetable *)frame_allocate();
    pagetable_init(p->pagetable);

    pagetable_alloc(
        p->pagetable,
        PROCESS_ENTRY_POINT,
        code_size,
        PAGE_FLAG_USER | PAGE_FLAG_READWRITE);

    pagetable_alloc(
        p->pagetable,
        PROCESS_STACK_INIT+0xF-stack_size+1,
        stack_size,
        PAGE_FLAG_USER | PAGE_FLAG_READWRITE);

    p->kernel_stack = (struct pagetable *)frame_allocate();
    p->entry = PROCESS_ENTRY_POINT;

    return p;
}

#include "task.h"
#include "paging.h"

extern page_directory_t *current_directory;

void
move_stack(
    void *new_stack_start,
    uint32_t size)
{
    uint32_t i;
    uint32_t start = (uint32_t)new_stack_start;
    uint32_t end = (uint32_t)new_stack_start - size;

    for (i = start; i >= end; i-= PAGE_SIZE)
    {
        alloc_frame(get_page(i, 1, current_directory), 0, 1);
    }

    flush_tlb();
}

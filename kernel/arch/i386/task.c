#include <string.h>

#include <newbos/heap.h>

#include "interrupts.h"
#include "paging.h"
#include "task.h"

volatile task_t *current_task;

volatile task_t *ready_queue;

/*
 * The next available process ID.
 */
uint32_t next_pid = 1;

extern page_directory_t *current_directory;

extern uint32_t initial_esp;

extern uint32_t get_esp();

extern uint32_t get_ebp();

extern uint32_t get_eip();

extern uint32_t set_esp();

extern uint32_t set_ebp();

void
init_tasks(
    void)
{
    disable_interrupts();

    /*
     * Relocate the stack so we know where it is.
     */
    // move_stack(0xE0000000, 0x2000); // FIXME!
    move_stack(0x80000, 0x2000);

    /*
     * Initialize the first task (kernel task).
     */
    current_task = ready_queue = (task_t *)kmalloc(sizeof(task_t));
    current_task->id = next_pid++;
    current_task->esp = current_task->ebp = 0;
    current_task->eip = 0;
    current_task->page_directory = current_directory;
    current_task->next = 0;

    enable_interrupts();
}

void
move_stack(
    uint32_t new_stack_start,
    uint32_t size)
{
    uint32_t i;
    uint32_t start = (uint32_t)new_stack_start;
    uint32_t end = (uint32_t)new_stack_start - size;

    for (i = start; i >= end; i-= PAGE_SIZE)
    {
        /*
         * General-purpose stack is in uesr-mode.
         */
        alloc_frame(get_page(i, 1, current_directory), 0, 1);
    }

    /*
     * Flush the TLB to inform the processor that a mapping in page table has
     * changed.
     */
    flush_tlb();

    /*
     * Get current stack/base pointers, and calculate an offset from old stack
     * to new new stack. We use offset to calculate new stack/base pointers.
     */
    uint32_t old_stack_pointer = get_esp();
    uint32_t old_base_pointer = get_ebp();

    uint32_t offset = new_stack_start - initial_esp;

    uint32_t new_stack_pointer = old_stack_pointer + offset;
    uint32_t new_base_pointer = old_base_pointer + offset;

    /*
     * Copy the stack.
     */
    memcpy((void *)new_stack_pointer, (void *)old_stack_pointer,
           initial_esp - old_stack_pointer);

    /*
     * Backtrace through the original stack, coyping new values into the new
     * stack. Assumes that any value on the stack in the range is a pushed EBP.
     * Any value in this range which isn't an EBP will be trashed.
     */
    for (i = new_stack_start; i > new_stack_start - size; i-= 4)
    {
        uint32_t tmp = *(uint32_t *)i;
        /*
         * If the value of tmp is inside the range of the old stack, assume it
         * is a base pointer and remap it.
         */
        if ((old_stack_pointer < tmp) && (tmp < initial_esp))
        {
            tmp = tmp + offset;
            uint32_t *tmp2 = (uint32_t *)i;
            *tmp2 = tmp;
        }
    }

    /*
     * Change stacks.
     */
    set_esp(new_stack_pointer);
    set_ebp(new_base_pointer);
}

int
fork(
    void)
{
    disable_interrupts();

    task_t *parent_task = (task_t *)current_task;

    page_directory_t *directory = clone_page_directory(current_directory);

    /*
     * Create a new process.
     */
    task_t *new_task = (task_t *)kmalloc(sizeof(task_t));
    new_task->id = next_pid++;
    new_task->esp = new_task->ebp = 0;
    new_task->eip = 0;
    new_task->page_directory = directory;
    new_task->next = 0;

    task_t *tmp_task = (task_t *)ready_queue;
    while (tmp_task->next)
    {
        tmp_task = tmp_task->next;
    }
    tmp_task->next = new_task;

    /*
     * We could be the parent or the child here - check.
     */
    if (current_task == parent_task)
    {
        new_task->esp = get_esp();
        new_task->esp = get_ebp();
        new_task->eip = get_eip();

        enable_interrupts();
        return new_task->id;
    }
    else
    {
        return 0;
    }
}

#include <stddef.h>

#include <newbos/kmalloc.h>
#include <newbos/scheduler.h>

/*
 * segements
 */
#define SEGSEL_KERNEL_CS 0x08
#define SEGSEL_KERNEL_DS 0x10
#define SEGSEL_USER_SPACE_CS 0x18
#define SEGSEL_USER_SPACE_DS 0x20

struct process_list_element {
    struct process_list_element *next;
    struct process *ps;
};

struct process_list {
    struct process_list_element *start;
    struct process_list_element *end;
};

static struct process_list runnable_processes = { NULL, NULL };

uint32_t
scheduler_next_pid(
    void)
{
    uint32_t max_pid = 0;
    struct process_list_element *p = runnable_processes.start;
    while (p != NULL)
    {
        if (p->ps != NULL && p->ps->id > max_pid)
        {
            max_pid = p->ps->id;
        }
        p = p->next;
    }

    return max_pid + 1;
}

int
scheduler_add_process(
    struct process *p)
{
    struct process_list_element *e = kmalloc(
        sizeof(struct process_list_element));
    if (e == NULL)
    {
        printk("scheduler_add_process: Couldn't allocate memory for element in "
              "process list\n");
        return -1;
    }

    e->ps = p;
    e->next = NULL;

    if (runnable_processes.start == NULL)
    {
        runnable_processes.start = e;
    }
    else
    {
        runnable_processes.end->next = e;
    }

    runnable_processes.end = e;
    return 0;
}

void
scheduler_schedule(
    void)
{
    if (runnable_processes.start == NULL ||
        runnable_processes.start->ps == NULL)
    {
        printk("scheduler: There are no runnable processes to schedule\n");
        return;
    }

    struct process_list_element *e = runnable_processes.start;
    if (e->next != NULL)
    {
        /*
         * More than one element in the list. Move current process (head of
         * list) to the end of list.
         */
        runnable_processes.start = e->next;
        e->next = NULL;
        runnable_processes.end->next = e;
        runnable_processes.end = e;
    }

    struct process *p = runnable_processes.start->ps;
    if (p == NULL)
    {
        printk("scheduler: No process to schedule\n");
        return;
    }

    tss_set_kernel_stack(SEGSEL_KERNEL_DS, p->kernel_stack_start_vaddr);
    pdt_load_process_pdt(p->pdt, p->pdt_paddr);

    if (p->current.cs == SEGSEL_KERNEL_CS) {
        // TODO: run_process_in_kernel_mode(&p->current);
    } else {
        run_process_in_user_mode(&p->current);
    }
}

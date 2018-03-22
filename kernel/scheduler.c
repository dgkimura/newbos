#include <stddef.h>

#include <newbos/kmalloc.h>
#include <newbos/scheduler.h>

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

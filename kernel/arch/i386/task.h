#ifndef _NEWBOS_TASK_H
#define _NEWBOS_TASK_H

#include <stdint.h>

#include "paging.h"

typedef struct task
{
    int32_t id;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    page_directory_t *page_directory;
} task_t;

void
init_tasking(
    void
);

void
switch_task(
    void
);

int fork(
    void
);

void
move_stack(
    uint32_t new_stack_start,
    uint32_t size
);

int32_t
getpid(
    void
);

#endif

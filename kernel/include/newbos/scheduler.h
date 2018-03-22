#ifndef _NEWBOS_SCHEDULER_H
#define _NEWBOS_SCHEDULER_H

#include <newbos/process.h>

uint32_t
scheduler_next_pid(
    void
);

int
scheduler_add_process(
    struct process *ps
);

void
scheduler_schedule(
    void
);

#endif

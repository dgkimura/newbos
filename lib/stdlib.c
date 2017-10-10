#include <stdlib.h>

#include <newbos/tty.h>

void
abort(void)
{
    monitor_write("abort()\n");
    for (;;);
}

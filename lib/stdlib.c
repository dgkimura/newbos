#include <stdlib.h>

#include <newbos/printk.h>

void
abort(void)
{
    printk("abort()\n");
    for (;;);
}

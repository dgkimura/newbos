#include <newbos/heap.h>
#include <newbos/timer.h>
#include <newbos/tty.h>

void
kernel_main(void)
{
    monitor_write("Welcome to newbos...\n");

    asm volatile ("int $0x3");
    asm volatile ("int $0x4");

    init_timer(1000);

    uint32_t *a = (uint32_t *)kmalloc(8);
    uint32_t *b = (uint32_t *)kmalloc(8);
    monitor_write("a: ");
    monitor_write_hex(*a);
    monitor_write("\n");
    monitor_write("b: ");
    monitor_write_hex(*b);
    monitor_write("\n");

    kfree(a);
    kfree(b);

    // Loop forever.
    for (;;);
}

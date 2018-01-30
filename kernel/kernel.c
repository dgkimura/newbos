#include <newbos/kmalloc.h>
#include <newbos/timer.h>
#include <newbos/tty.h>

void
kernel_main(void)
{
    monitor_write("Welcome to newbos...\n");

    kmalloc_init((void *)KMALLOC_START, KMALLOC_LENGTH);

    //asm volatile ("int $0x3");
    //asm volatile ("int $0x4");

    init_timer(1000);

    // Loop forever.
    for (;;);
}

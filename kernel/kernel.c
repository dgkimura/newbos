#include <newbos/kmalloc.h>
#include <newbos/paging.h>
#include <newbos/timer.h>
#include <newbos/tty.h>

#include "keyboard.h"

void
kernel_main(void)
{
    monitor_write("Welcome to newbos...\n");

    frames_init();

    kmalloc_init((void *)KMALLOC_START, KMALLOC_LENGTH);

    //asm volatile ("int $0x3");
    //asm volatile ("int $0x4");

    keyboard_init();

    timer_init(1000);

    // Loop forever.
    for (;;);
}

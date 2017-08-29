#include <newbos/timer.h>
#include <newbos/tty.h>

#if !defined(__i386__)
#error "Must be compiled with an ix86-elf compiler"
#endif

void
kernel_main(void)
{
    /* Initialize terminal interface */
    monitor_clear();

    /* Newline support is left as an exercise. */
    monitor_write("Hello, kernel World!\n");

    asm volatile ("int $0x3");
    asm volatile ("int $0x4");

    init_timer(50);
}

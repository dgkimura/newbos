#include <newbos/timer.h>
#include <newbos/tty.h>

#if !defined(__i386__)
#error "Must be compiled with an ix86-elf compiler"
#endif

void
kernel_main(void)
{
    asm volatile ("int $0x3");
    asm volatile ("int $0x4");

    init_timer(1000);

    // Loop forever.
    for (;;);
}

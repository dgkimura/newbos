#include <newbos/tty.h>

#if !defined(__i386__)
#error "Must be compiled with an ix86-elf compiler"
#endif

extern void init_gdt();
extern void init_idt();

void
kernel_main(void)
{
    init_gdt();
    init_idt();

    /* Initialize terminal interface */
    monitor_clear();

    /* Newline support is left as an exercise. */
    monitor_write("Hello, kernel World!\n");

    asm volatile ("int $0x3");
    asm volatile ("int $0x4");
}

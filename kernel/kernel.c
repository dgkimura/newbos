#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <newbos/tty.h>
#include <newbos/gdt.h>

#if !defined(__i386__)
#error "Must be compiled with an ix86-elf compiler"
#endif

void kernel_main(void)
{
	gdt_install();

    /* Initialize terminal interface */
    terminal_initialize();

    /* Newline support is left as an exercise. */
    terminal_writestring("Hello, kernel World!\n");
}

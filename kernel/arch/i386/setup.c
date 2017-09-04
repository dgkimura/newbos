#include "gdt.h"
#include "idt.h"

void
kernel_setup(void)
{
    init_gdt();
    init_idt();
}

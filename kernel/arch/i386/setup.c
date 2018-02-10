#include <newbos/tty.h>

#include "gdt.h"
#include "interrupts.h"
#include "multiboot.h"

void
kernel_setup(multiboot_t *multiboot)
{
    monitor_clear();

    init_gdt();
    clear_idt();

    interrupts_init();

    enable_interrupts();
}

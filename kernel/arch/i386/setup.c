#include <newbos/tty.h>

#include "gdt.h"
#include "interrupts.h"

extern void idt_flush(uint32_t);

extern void enable_interrupts();

void
kernel_setup(void)
{
    monitor_clear();

    init_gdt();
    clear_idt();

    init_isr();
    init_irq();

    idt_flush((uint32_t)&idt_ptr);

    enable_interrupts();
    monitor_write("Interrupts enabled.\n");
}

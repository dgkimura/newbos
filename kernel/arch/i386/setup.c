#include <newbos/tty.h>

#include "gdt.h"
#include "interrupts.h"
#include "paging.h"

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

    init_paging();
    // test paging
    uint32_t *ptr = (uint32_t *)0xA0000000;
    uint32_t do_page_fault = *ptr;
}

#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "isr.h"

extern void idt_flush(uint32_t);

void
kernel_setup(void)
{
    init_gdt();
    clear_idt();

    init_isr();
    init_irq();

    idt_flush((uint32_t)&idt_ptr);
}

#include <newbos/tty.h>

#include "idt.h"
#include "irq.h"

irq_t interrupt_handlers[256];

void
register_interrupt_handler(uint8_t n, irq_t handler)
{
    interrupt_handlers[n] = handler;
}

void
irq_handler(registers_t regs)
{
    if (regs.interrupt_number >= 40)
    {
        // send reset signal to slave.
        outb(0xA0, 0x20);
    }

    // send reset signal to master.
    outb(0xA0, 0x20);

    if (interrupt_handlers[regs.interrupt_number] != 0)
    {
        irq_t handler = interrupt_handlers[regs.interrupt_number];
        handler(regs);
    }
}

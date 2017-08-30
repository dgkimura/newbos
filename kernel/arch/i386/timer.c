#include "idt.h"
#include "irq.h"

#include <newbos/timer.h>
#include <newbos/tty.h>

uint32_t tick = 0;

static void
timer_callback(registers_t regs)
{
    tick++;
    monitor_write("Tick: ");
    monitor_write_dec(tick);
    monitor_write("\n");
}

void
init_timer(uint32_t frequency)
{
    register_irq_handler(IRQ0, &timer_callback);

    // The value we send to the PIT is the value to divide it's input clock
    // (1193180 Hz) by, to get our required frequency. Important to note is
    // that the divisor must be small enough to fit into 16-bits.
    uint32_t divisor = 1193180 / frequency;

    // send command byte.
    outb(0x43, 0x36);

    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

    // send the frequency divisor.
    outb(0x40, l);
    outb(0x40, h);
}

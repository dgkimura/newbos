#include <newbos/tty.h>

#include "gdt.h"
#include "interrupts.h"
#include "keyboard.h"
#include "multiboot.h"

extern void idt_flush(uint32_t);

void
kernel_setup(multiboot_t *multiboot)
{
    monitor_clear();

    init_gdt();
    clear_idt();

    init_isr();
    init_irq();

    idt_flush((uint32_t)&idt_ptr);

    enable_interrupts();
    monitor_write("Interrupts enabled.\n");

    init_keyboard();
}

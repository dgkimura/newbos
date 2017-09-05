#include <string.h>

#include <newbos/tty.h>

#include "idt.h"

idt_entry_t idt_entries[256];

void
idt_set_gate(uint8_t number, uint32_t base, uint16_t selector, uint8_t flags)

{
    idt_entries[number].base_lo = base & 0xFFFF;
    idt_entries[number].base_hi = (base >> 16) & 0xFFFF;

    idt_entries[number].selector = selector;
    idt_entries[number].always0 = 0;
    // We must uncomment the OR below when we get to using user-mode.
    // It sets the interrupt gate's privilege level to 3.
    idt_entries[number].flags = flags | 0x60;
}

void
clear_idt()
{
    idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    memset(&idt_entries, 0, sizeof(idt_entry_t)*256);
}

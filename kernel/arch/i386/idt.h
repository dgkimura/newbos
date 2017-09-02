#ifndef _NEWBOS_IDT_H
#define _NEWBOS_IDT_H

#include <stdint.h>

// A struct describing an interrupt gate.
struct idt_entry_struct
{
    uint16_t base_lo;   // The lower 16 bits of the address to jump to when this interrupt fires.
    uint16_t selector;  // Kernel segment selector.
    uint8_t  always0;   // This must always be zero.
    uint8_t  flags;     // More flags. See documentation.
    uint16_t base_hi;   // The upper 16 bits of the address to jump to.
} __attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct idt_ptr_struct
{
    uint16_t limit;
    uint32_t base;      // The address of the first element in our idt_entry_t array.
} __attribute__((packed));

typedef struct idt_ptr_struct idt_ptr_t;

void init_idt();

typedef struct registers
{
    // Data segment selector
    uint32_t ds;

    // Pushed by pusha.
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

    // Interrupt number and error code (if applicable)
    uint32_t error_code, interrupt_number;

    // Pushed by the processor automatically.
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

extern void interrupt_handler(registers_t regs);

void register_interrupt_handler(int number, void (*handler)(registers_t));

#endif

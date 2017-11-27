#ifndef _NEWBOS_INTERRUPTS_H
#define _NEWBOS_INTERRUPTS_H

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

idt_ptr_t   idt_ptr;

void clear_idt();

typedef struct registers
{
    // Data segment selector
    uint32_t gs, fs, es, ds;

    // Pushed by pusha.
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

    // Interrupt number and error code (if applicable)
    uint32_t interrupt_number, error_code;

    // Pushed by the processor automatically.
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

void idt_set_gate(uint8_t number, uint32_t base, uint16_t selector, uint8_t flags);

void init_isr();

void interrupt_handler(registers_t* regs);

void register_isr_handler(int number, void (*handler)(registers_t*));

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

typedef void (*irq_t)(registers_t*);

void register_irq_handler(uint8_t, irq_t handler);

void irq_handler(registers_t* regs);

void init_irq();

void enable_interrupts();

void disable_interrupts();

#endif

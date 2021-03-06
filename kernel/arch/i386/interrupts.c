#include <stdlib.h>
#include <string.h>

#include <newbos/printk.h>

#include "interrupts.h"
#include "io.h"

idt_entry_t idt_entries[256];

extern void idt_flush(uint32_t);

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

typedef void (*isr_t)(registers_t*);

isr_t exception_handlers[32];

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

char* exception_messages[] =
{
    "Divide Error",
    "Debug Exceptions",
    "NMI Interrupt",
    "Breakpoint",
    "INTO Detected Overflow",
    "BOUND Range Exceeded",
    "Invalid Opcode",
    "Coprocessor Not Available",
    "Double Exception",
    "Coprocessor Segment Overrun",
    "Invalid Task State Segment",
    "Segment Not Present",
    "Stack Fault",
    "General Protection",
    "Page Fault",
    "(reserved)",
    "Coprocessor Error",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)"
};

static void
init_isr()
{
    memset(&exception_handlers, 0, sizeof(isr_t)*32);

    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);
}

void
interrupt_handler(registers_t* regs)
{
    if (0 != exception_handlers[regs->interrupt_number])
    {
        exception_handlers[regs->interrupt_number](regs);
    }
    else
    {
        printk("Unhandled exception - %X : [errno - %X]: %s\n"
               "Data segments: gs[%X], fs[%X], es[%X], ds[%X]\n"
               "edi: %X\n"
               "esi: %X\n"
               "ebp: %X\n"
               "esp: %X\n"
               "ebx: %X\n"
               "edx: %X\n"
               "ecx: %X\n"
               "eax: %X\n"
               "eip: %X\n"
               "cs: %X\n"
               "eflags: %X\n"
               "usersp: %X\n"
               "ss: %X\n",
               regs->interrupt_number,
               regs->error_code,
               exception_messages[regs->interrupt_number],
               regs->gs,
               regs->fs,
               regs->es,
               regs->ds,
               regs->edi,
               regs->esi,
               regs->ebp,
               regs->esp,
               regs->ebx,
               regs->edx,
               regs->ecx,
               regs->eax,
               regs->eip,
               regs->cs,
               regs->eflags,
               regs->useresp,
               regs->ss);
        abort();
    }
}

void
register_isr_handler(
    int number,
    void (*handler)(registers_t*))
{
    exception_handlers[number] = handler;
}

irq_t interrupt_handlers[256];

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

static void
init_irq()
{
    // Remap the IRQ table.
    //   Master - command: 0x20, data: 0x21
    //   Slave - command: 0xA0, data: 0xA1
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
}

void
register_irq_handler(uint8_t n, irq_t handler)
{
    interrupt_handlers[n] = handler;
}

void
irq_handler(registers_t* regs)
{
    if (regs->interrupt_number >= 40)
    {
        // send reset signal to slave.
        outb(0xA0, 0x20);
    }

    // send reset signal to master.
    outb(0x20, 0x20);

    if (interrupt_handlers[regs->interrupt_number] != 0)
    {
        irq_t handler = interrupt_handlers[regs->interrupt_number];
        handler(regs);
    }
}

void
interrupts_init()
{
    init_isr();
    init_irq();

    idt_flush((uint32_t)&idt_ptr);

    printk("Interrupts enabled.\n");
}

#ifndef _NEWBOS_ISR_H
#define _NEWBOS_ISR_H

void init_isr();

extern void interrupt_handler(registers_t regs);

void register_isr_handler(int number, void (*handler)(registers_t));

#endif

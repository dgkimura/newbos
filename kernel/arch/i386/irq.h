#include <stdint.h>

typedef void (*irq_t)(registers_t);

void register_interrupt_handler(uint8_t, irq_t handler);

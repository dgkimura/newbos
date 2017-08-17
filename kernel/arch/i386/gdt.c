#include <stdint.h>

/* Defines a GDT entry. */
struct gdt_entry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t grandularity;
    uint8_t base_high;
} __attribute__((packed));

/* Special pointer which includes the limit: The max bytes taken up bythe GDT,
   minus 1. */
struct gdt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct gdt_entry GDT[]= 
{
};

void gdt_install()
{
}

#include "gdt.h"

#define SEGMENT_BASE    0
#define SEGMENT_LIMIT   0xFFFFF

#define PL0 0x0
#define PL3 0x3

#define CODE_RX_TYPE    0xA
#define DATA_RW_TYPE    0x2

#define GDT_NUM_ENTRIES 5

/*
 * Lets us access our ASM functions from our C code.
 */
void gdt_flush(uint32_t);

static void gdt_set_gate(uint32_t num, uint8_t plevel, uint8_t type);

/*
 * This structure contains the value of one GDT entry.
 * We use the attribute 'packed' to tell GCC not to change
 * any of the alignment in the structure.
 */
struct gdt_entry
{
    uint16_t limit_low;     /* The lower 16 bits of the limit */
    uint16_t base_low;      /* The lower 16 bits of the base */
    uint8_t  base_mid;      /* The next 8 bits of the base */
    uint8_t  access;        /* Contains access flags */
    uint8_t  granularity;   /* Specify granularity, and 4 bits of limit */
    uint8_t  base_high;     /* The last 8 bits of the base; */
} __attribute__((packed));  /* It needs to be packed like this, 64 bits */

/*
 * This struct describes a GDT pointer. It points to the start of
 * our array of GDT entries, and is in the format required by the
 * lgdt instruction.
 */
struct gdt_ptr
{
    uint16_t limit;          /* Size of gdt table in bytes*/
    uint32_t base;           /* Address to the first gdt entry */
} __attribute__((packed));

struct gdt_entry gdt_entries[GDT_NUM_ENTRIES];

void gdt_init()
{
	struct gdt_ptr gdt_ptr;
    gdt_ptr.limit = sizeof(struct gdt_entry) * GDT_NUM_ENTRIES;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    gdt_set_gate(0, 0, 0);              /* Null segment */
    gdt_set_gate(1, PL0, CODE_RX_TYPE); /* Code segment */
    gdt_set_gate(2, PL0, DATA_RW_TYPE); /* Data segment */
    gdt_set_gate(3, PL3, CODE_RX_TYPE); /* User mode code segment */
    gdt_set_gate(4, PL3, DATA_RW_TYPE); /* User mode data segment */

    gdt_flush((uint32_t)&gdt_ptr);
}

/*
 * Set the value of one GDT entry.
 */
static void
gdt_set_gate(uint32_t num, uint8_t plevel, uint8_t type)
{
    gdt_entries[num].base_low = (SEGMENT_BASE & 0xFFFF);
    gdt_entries[num].base_mid = (SEGMENT_BASE >> 16) & 0xFF;
    gdt_entries[num].base_high = (SEGMENT_BASE >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (SEGMENT_LIMIT & 0xFFFF);

    /*
     * name | value | size | desc
     * ---------------------------
     * G    |     1 |    1 | granularity, size of segment unit, 1 = 4kB
     * D/B  |     1 |    1 | size of operation size, 0 = 16 bits, 1 = 32 bits
     * L    |     0 |    1 | 1 = 64 bit code
     * AVL  |     0 |    1 | "available for use by system software"
     * LIM  |   0xF |    4 | the four highest bits of segment limit
     */
    gdt_entries[num].granularity |= (0x01 << 7) | (0x01 << 6) | 0x0F;

    /*
     * name | value | size | desc
     * ---------------------------
     * P    |     1 |    1 | segment present in memory
     * DPL  |    pl |    2 | privilege level
     * S    |     1 |    1 | descriptor type, 0 = system, 1 = code or data
     * Type |  type |    4 | segment type, how the segment can be accessed
     */
    gdt_entries[num].access =
        (0x01 << 7) |((plevel & 0x03) << 5) | (0x01 << 4) | (type & 0x0F);
}

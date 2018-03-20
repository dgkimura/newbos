#include <stdint.h>
#include <string.h>

#include <newbos/paging.h>
#include <newbos/printk.h>

#include "memory.h"

#define NUM_ENTRIES 1024
#define PDT_SIZE NUM_ENTRIES * sizeof(struct pde)
#define MAX_NUM_MEMORY_MAP  100
#define FOUR_KB     0x1000
#define PAGING_PL0        0
#define PS_4KB 0x00

#define VIRTUAL_TO_PDT_IDX(a)   (((a) >> 22) & 0x3FF)
#define VIRTUAL_TO_PT_IDX(a)    (((a) >> 12) & 0x3FF)
#define PDT_IDX_TO_VIRTUAL(a)   (((a) << 22))
#define PT_IDX_TO_VIRTUAL(a)   (((a) << 12))
#define KERNEL_START_VADDR  0xC0000000
#define KERNEL_TMP_PT_IDX   1023
#define PT_ENTRY_SIZE  FOUR_KB
#define KERNEL_TMP_VADDR \
    (KERNEL_START_VADDR + KERNEL_TMP_PT_IDX * PT_ENTRY_SIZE)
#define KERNEL_PT_PDT_IDX VIRTUAL_TO_PDT_IDX(KERNEL_START_VADDR)

#define IS_ENTRY_PRESENT(e) ((e)->config && 0x01)

#define PAGING_READ_WRITE 1

struct pte
{
    uint8_t config;
    uint8_t middle; /* only the highest 4 bits and the lowest bit are used */
    uint16_t high_addr;
} __attribute__((packed));

static struct pde *kernel_pdt;
static struct pte *kernel_pt;

struct memory_map
{
    uint32_t addr;
    uint32_t len;
};

struct page_frame_bitmap {
    uint32_t *start;
    uint32_t len; /* in bits */
};
static struct page_frame_bitmap page_frames;
static struct memory_map mmap[MAX_NUM_MEMORY_MAP];
static uint32_t mmap_len;

static void
pfa_free(uint32_t paddr);

static void
toggle_bit(uint32_t bit_idx);


static void
create_pdt_entry(struct pde *pdt, uint32_t n, uint32_t paddr, uint8_t ps,
                 uint8_t rw, uint8_t pl);

static void
create_pt_entry(struct pte *pt, uint32_t n, uint32_t paddr, uint8_t rw,
                uint8_t pl);

static uint32_t
idx_for_paddr(uint32_t paddr);

static uint32_t
align_up(
    uint32_t n,
    uint32_t a)
{
    uint32_t m = n % a;
    if (m == 0)
    {
        return n;
    }
    return n + (a - m);
}

static uint32_t
align_down(
    uint32_t n,
    uint32_t a)
{
    return n - (n % a);
}

static uint32_t
div_ceil(
    uint32_t num,
    uint32_t den)
{
    return (num - 1) / den + 1;
}

static uint32_t
get_pt_paddr(
    struct pde *pde,
    uint32_t pde_idx)
{
    struct pde *e = pde + pde_idx;
    uint32_t addr = e->high_addr;
    addr <<= 16;
    addr |= ((uint32_t) (e->low_addr & 0xF0) << 8);

    return addr;
}

static uint32_t
kernel_map_temporary_memory(
    uint32_t paddr)
{
    create_pt_entry(kernel_pt, KERNEL_TMP_PT_IDX, paddr,
                    PAGING_READ_WRITE, PAGING_PL0);
    invalidate_page_table_entry(KERNEL_TMP_VADDR);
    return KERNEL_TMP_VADDR;
}

static void
kernel_set_temporary_entry(
    uint32_t entry)
{
    kernel_pt[KERNEL_TMP_PT_IDX] = *((struct pte *) &entry);
    invalidate_page_table_entry(KERNEL_TMP_VADDR);
}

static uint32_t
kernel_get_temporary_entry()
{
    return *((uint32_t *) &kernel_pt[KERNEL_TMP_PT_IDX]);
}

struct pde *
pdt_create(uint32_t *out_paddr)
{
    struct pde *pdt;
    *out_paddr = 0;
    uint32_t pdt_paddr = pfa_allocate(1);
    uint32_t pdt_vaddr = pdt_kernel_find_next_vaddr(PDT_SIZE);
    uint32_t size = pdt_map_kernel_memory(pdt_paddr, pdt_vaddr, PDT_SIZE,
                                          PAGING_READ_WRITE, PAGING_PL0);
    if (size < PDT_SIZE) {
        /* Since PDT_SIZE is the size of one frame, size must either be equal
         * to PDT_SIZE or 0
         */
        pfa_free(pdt_paddr);
        return NULL;
    }

    pdt = (struct pde *) pdt_vaddr;

    memset(pdt, 0, PDT_SIZE);

    *out_paddr = pdt_paddr;
    return pdt;
}

static uint32_t
pt_kernel_find_next_vaddr(
    uint32_t pdt_idx,
    struct pte *pt, uint32_t size)
{
    uint32_t i, num_to_find, num_found = 0, org_i;
    num_to_find = align_up(size, FOUR_KB) / FOUR_KB;

    for (i = 0; i < NUM_ENTRIES; ++i) {
        if (IS_ENTRY_PRESENT(pt+i) ||
            (pdt_idx == KERNEL_PT_PDT_IDX && i == KERNEL_TMP_PT_IDX)) {
            num_found = 0;
        } else {
            if (num_found == 0) {
                    org_i = i;
            }
            ++num_found;
            if (num_found == num_to_find) {
                return PDT_IDX_TO_VIRTUAL(pdt_idx) |
                       PT_IDX_TO_VIRTUAL(org_i);
            }
        }
    }
    return 0;
}

uint32_t
pdt_kernel_find_next_vaddr(
    uint32_t size)
{
    uint32_t pdt_idx, pt_paddr, pt_vaddr, tmp_entry, vaddr = 0;
    /*
     * TODO: support > 4MB sizes
     */

    pdt_idx = VIRTUAL_TO_PDT_IDX(KERNEL_START_VADDR);
    for (; pdt_idx < NUM_ENTRIES; ++pdt_idx)
    {
        if (IS_ENTRY_PRESENT(kernel_pdt + pdt_idx))
        {
            tmp_entry = kernel_get_temporary_entry();

            pt_paddr = get_pt_paddr(kernel_pdt, pdt_idx);
            pt_vaddr = kernel_map_temporary_memory(pt_paddr);
            vaddr =
                pt_kernel_find_next_vaddr(pdt_idx, (struct pte *) pt_vaddr, size);

            kernel_set_temporary_entry(tmp_entry);
        }
        else
        {
            /*
             * No pdt entry
             */
            vaddr = PDT_IDX_TO_VIRTUAL(pdt_idx);
        }
        if (vaddr != 0)
        {
            return vaddr;
        }
    }

    return 0;
}

static uint32_t pt_map_memory(
    struct pte *pt,
    uint32_t pdt_idx,
    uint32_t paddr,
    uint32_t vaddr,
    uint32_t size,
    uint8_t rw,
    uint8_t pl)
{
    uint32_t pt_idx = VIRTUAL_TO_PT_IDX(vaddr);
    uint32_t mapped_size = 0;

    while (mapped_size < size && pt_idx < NUM_ENTRIES)
    {
        if (IS_ENTRY_PRESENT(pt + pt_idx))
        {
            printk("Entry is present: pt: %X, pt_idx: %u, pdt_idx: %u "
                   "pt[pt_idx]: %X\n",
                   pt, pt_idx, pdt_idx, pt[pt_idx]);
            return mapped_size;
        } else if(pdt_idx == KERNEL_PT_PDT_IDX && pt_idx == KERNEL_TMP_PT_IDX) {
            return mapped_size;
        }

        create_pt_entry(pt, pt_idx, paddr, rw, pl);

        paddr += PT_ENTRY_SIZE;
        mapped_size += PT_ENTRY_SIZE;
        ++pt_idx;
    }

    return mapped_size;
}

static uint32_t
pdt_map_memory(
    struct pde *pdt,
    uint32_t paddr,
    uint32_t vaddr,
    uint32_t size,
    uint8_t rw,
    uint8_t pl)
{
    uint32_t pdt_idx;
    struct pte *pt;
    uint32_t pt_paddr, pt_vaddr, tmp_entry;
    uint32_t mapped_size = 0;
    uint32_t total_mapped_size = 0;
    size = align_up(size, PT_ENTRY_SIZE);

    while (size != 0)
    {
        pdt_idx = VIRTUAL_TO_PDT_IDX(vaddr);

        tmp_entry = kernel_get_temporary_entry();

        if (!IS_ENTRY_PRESENT(pdt + pdt_idx)) {
            pt_paddr = pfa_allocate(1);
            if (pt_paddr == 0) {
                printk("Couldn't allocate page frame for new page table."
                       "pdt_idx: %u, data vaddr: %X, data paddr: %X, "
                       "data size: %u\n",
                       pdt_idx, vaddr, paddr, size);
                return 0;
            }
            pt_vaddr = kernel_map_temporary_memory(pt_paddr);
            memset((void *) pt_vaddr, 0, NUM_ENTRIES * sizeof(struct pte));
        } else {
            pt_paddr = get_pt_paddr(pdt, pdt_idx);
            pt_vaddr = kernel_map_temporary_memory(pt_paddr);
        }

        pt = (struct pte *) pt_vaddr;
        mapped_size =
            pt_map_memory(pt, pdt_idx, paddr, vaddr, size, rw, pl);

        if (mapped_size == 0) {
            printk("Could not map memory in page table. "
                   "pt: %X, paddr: %X, vaddr: %X, size: %u\n",
                   (uint32_t) pt, paddr, vaddr, size);
            kernel_set_temporary_entry(tmp_entry);
            return 0;
        }

        if (!IS_ENTRY_PRESENT(pdt + pdt_idx)) {
            create_pdt_entry(pdt, pdt_idx, pt_paddr, PS_4KB, rw, pl);
        }

        kernel_set_temporary_entry(tmp_entry);

        size -= mapped_size;
        total_mapped_size += mapped_size;
        vaddr += mapped_size;
        paddr += mapped_size;
    }

    return total_mapped_size;
}

uint32_t
pdt_map_kernel_memory(
    uint32_t paddr,
    uint32_t vaddr,
    uint32_t size,
    uint8_t rw,
    uint8_t pl)
{
    return pdt_map_memory(kernel_pdt, paddr, vaddr,
                          size, rw, pl);
}

static uint32_t fill_memory_map(
    uint32_t kernel_physical_start,
    uint32_t kernel_physical_end,
    uint32_t kernel_virtual_start,
    uint32_t kernel_virtual_end,
    struct multiboot_info *multiboot_info)
{
    uint32_t addr = 0, len = 0, i = 0;

    /*
     * Grub multiboot documents that flag[6] bit indicates presense of mmap_*
     * fields set in multiboot structure.
     */
    if ((multiboot_info->flags & 0x00000040) == 0)
    {
        printk("No memory map from GRUB\n");
        return 0;
    }

    multiboot_memory_map_t *entry =
        (multiboot_memory_map_t *) multiboot_info->mmap_addr;
    while ((uint32_t) entry < multiboot_info->mmap_addr +
                              multiboot_info->mmap_length)
    {
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            addr = (uint32_t) entry->addr;
            len = (uint32_t) entry->len;
            if (addr <= kernel_physical_start
                    && (addr + len) > kernel_physical_end)
            {

                addr = kernel_physical_end;
                len = len - kernel_physical_end;

            }

            if (addr > 0x100000)
            {
                mmap[i].addr = addr;
                mmap[i].len = len;
                ++i;
            }
        }
        entry = (multiboot_memory_map_t *)
            (((uint32_t) entry) + entry->size + sizeof(entry->size));
    }

    return i;
}

static uint32_t
construct_bitmap(struct memory_map *mmap, uint32_t n)
{
    uint32_t i, bitmap_pfs, bitmap_size, paddr, vaddr, mapped_mem;
    uint32_t total_pfs = 0;

    /*
     * Calculate number of available page frames.
     */
    for (i = 0; i < n; ++i)
    {
        total_pfs += mmap[i].len / FOUR_KB;
    }

    bitmap_pfs = div_ceil(div_ceil(total_pfs, 8), FOUR_KB);

    for (i = 0; i < n; ++i)
    {
        if (mmap[i].len >= bitmap_pfs * FOUR_KB)
        {
            paddr = mmap[i].addr;

            mmap[i].addr += bitmap_pfs * FOUR_KB;
            mmap[i].len -= bitmap_pfs * FOUR_KB;
            break;
        }
    }

    page_frames.len = total_pfs - bitmap_pfs;
    bitmap_size = div_ceil(page_frames.len, 8);

    if (i == n)
    {
        printk("Couldn't find place for bitmap. bitmap_size: %u\n",
               bitmap_size);
        return 1;
    }

    vaddr = pdt_kernel_find_next_vaddr(bitmap_size);
    if (vaddr == 0)
    {
        printk("Could not find virtual address for bitmap in kernel. "
               "paddr: %X, bitmap_size: %u, bitmap_pfs: %u\n",
               paddr, bitmap_size);
        return 1;

    }

    mapped_mem = pdt_map_kernel_memory(paddr, vaddr, bitmap_size,
                                       PAGING_PL0, PAGING_READ_WRITE);
    if (mapped_mem < bitmap_size) {
        printk("Could not map kernel memory for bitmap. "
               "paddr: %X, vaddr: %X, bitmap_size: %u\n",
               paddr, vaddr, bitmap_size);
        return 1;
    }

    page_frames.start = (uint32_t *) vaddr;

    memset(page_frames.start, 0xFF, bitmap_size);
    uint8_t *last = (uint8_t *)((uint32_t)page_frames.start + bitmap_size - 1);
    *last = 0;
    for (i = 0; i < page_frames.len % 8; ++i)
    {
        *last |= 0x01 << (7 - i);
    }


    return 0;
}
void
frames_init(
    uint32_t kernel_physical_start,
    uint32_t kernel_physical_end,
    uint32_t kernel_virtual_start,
    uint32_t kernel_virtual_end,
    uint32_t kernel_pdt_vaddr,
    uint32_t kernel_pt_vaddr,
    struct multiboot_info *minfo)
{
    uint32_t i, addr, len;

    printk("Kernel Address:\n");
    printk(" Physical: [%X ... %X]\n",
           kernel_physical_start, kernel_physical_end);
    printk(" Virtual:  [%X ... %X]\n",
           kernel_virtual_start, kernel_virtual_end);

    kernel_pdt =  kernel_pdt_vaddr,
    kernel_pt = kernel_pt_vaddr,

    mmap_len = fill_memory_map(
        kernel_physical_start,
        kernel_physical_end,
        kernel_virtual_start,
        kernel_virtual_end,
        minfo);

    for (i = 0; i < mmap_len; ++i) {
        /*
         * Align addresses on 4KB blocks
         */
        addr = align_up(mmap[i].addr, FOUR_KB);
        len = align_down(mmap[i].len - (addr - mmap[i].addr), FOUR_KB);

        mmap[i].addr = addr;
        mmap[i].len = len;
    }

    construct_bitmap(mmap, mmap_len);
}

static void
pfa_free(uint32_t paddr)
{
    uint32_t bit_idx = idx_for_paddr(paddr);
    if (bit_idx == page_frames.len) {
        printk("pfa_free: invalid paddr %X\n", paddr);
    } else {
        toggle_bit(bit_idx);
    }
}

static void
toggle_bit(uint32_t bit_idx)
{
    uint32_t *bits = page_frames.start;
    bits[bit_idx/32] ^= (0x01 << (31 - (bit_idx % 32)));
}

static void
toggle_bits(uint32_t bit_idx, uint32_t num_bits)
{
    uint32_t i;
    for (i = bit_idx; i < bit_idx + num_bits; ++i) {
        toggle_bit(i);
    }
}

static uint32_t
idx_for_paddr(uint32_t paddr)
{
    uint32_t i, byte_offset = 0;
    for (i = 0; i < mmap_len; ++i) {
        if (paddr < mmap[i].addr + mmap[i].len) {
            byte_offset += paddr - mmap[i].addr;
            return byte_offset / FOUR_KB;
        } else {
            byte_offset += mmap[i].len;
        }
    }

    return page_frames.len;
}

static uint32_t
fits_in_one_mmap_entry(
    uint32_t bit_idx,
    uint32_t pfs)
{
    uint32_t i, current_offset = 0, offset = bit_idx * FOUR_KB;
    for (i = 0; i < mmap_len; ++i) {
        if (current_offset + mmap[i].len <= offset) {
            current_offset += mmap[i].len;
        } else {
            offset -= current_offset;
            if (offset + pfs * FOUR_KB <= mmap[i].len) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    return 0;
}

static uint32_t
paddr_for_idx(
    uint32_t bit_idx)
{
    uint32_t i, current_offset = 0, offset = bit_idx * FOUR_KB;
    for (i = 0; i < mmap_len; ++i) {
        if (current_offset + mmap[i].len <= offset) {
            current_offset += mmap[i].len;
        } else {
            offset -= current_offset;
            return mmap[i].addr + offset;
        }
    }

    return 0;
}

uint32_t
pfa_allocate(
    uint32_t num_page_frames)
{
    uint32_t i, j, cell, bit_idx;
    uint32_t n = div_ceil(page_frames.len, 32), frames_found = 0;

    for (i = 0; i < n; ++i) {
        cell = page_frames.start[i];
        if (cell != 0) {
            for (j = 0; j < 32; ++j) {
                if (((cell >> (31 - j)) & 0x1) == 1) {
                    if (frames_found == 0) {
                        bit_idx = i * 32 + j;
                    }
                    ++frames_found;
                    if (frames_found == num_page_frames) {
                        if (fits_in_one_mmap_entry(bit_idx, num_page_frames)) {
                            toggle_bits(bit_idx, num_page_frames);
                            return paddr_for_idx(bit_idx);
                        } else {
                            frames_found = 0;
                        }
                    }
                } else {
                    frames_found = 0;
                }
            }
        } else {
            frames_found = 0;
        }
    }

    return 0;
}

/**
 * Creates an entry in the page descriptor table at the specified index.
 * THe entry will point to the given PTE.
 *
 * @param pdt   The page descriptor table
 * @param n     The index in the PDT
 * @param addr  The address to the first entry in the page table, or a
 *              4MB page frame
 * @param ps    Page size, either PS_4KB or PS_4MB
 * @param rw    Read/write permission, 0 = read-only, 1 = read and write
 * @param pl    The required privilege level to access the page,
 *              0 = PL0, 1 = PL3
 */
static void
create_pdt_entry(
    struct pde *pdt,
    uint32_t n,
    uint32_t addr,
    uint8_t ps,
    uint8_t rw,
    uint8_t pl)
{
    /* Since page tables are aligned at 4kB boundaries, we only need to store
     * the 20 highest bits */
    /* The lower 4 bits */
    pdt[n].low_addr  = ((addr >> 12) & 0xF) << 4;
    pdt[n].high_addr = ((addr >> 16) & 0xFFFF);

    /*
     * name    | value | size | desc
     * ---------------------------
     *       P |     1 |    1 | If the entry is present or not
     *     R/W |    rw |    1 | Read/Write:
     *                              0 = Read-only
     *                              1 = Write and Read
     *     U/S |    pl |    1 | User/Supervisor:
     *                              0 = PL3 can't access
     *                              1 = PL3 can access
     *     PWT |     1 |    1 | Page-level write-through:
     *                              0 = writes are cached
     *                              1 = writes are not cached
     *     PCD |     0 |    1 | Page-level cache disable
     *       A |     0 |    1 | Is set if the entry has been accessed
     * Ignored |     0 |    1 | Ignored
     *      PS |    ps |    1 | Page size:
     *                              0 = address point to pt entry,
     *                              1 = address points to 4 MB page
     * Ignored |     0 |    4 | Ignored
     *
     * NOTE: Ignored is not part of pdt[n].config!
     */
    pdt[n].config =
        ((ps & 0x01) << 7) | (0x01 << 3) | ((pl & 0x01) << 2) |
        ((rw & 0x01) << 1) | 0x01;
}

/*
 * Creates a new entry at the specified index in the given page table.
 *
 * @param pt    The page table
 * @param n     The index in the page table to create the entry at
 * @param addr  The addres to the page frame
 * @param rw    Read/write permission, 0 = read-only, 1 = read and write
 * @param pl    The required privilege level to access the page,
 *              0 = PL0, 1 = PL3
 */
static void
create_pt_entry(
    struct pte *pt,
    uint32_t n,
    uint32_t addr,
    uint8_t rw,
    uint8_t pl)
{
    /* Since page tables are aligned at 4kB boundaries, we only need to store
     * the 20 highest bits */
    /* The lower 4 bits */
    pt[n].middle  = ((addr >> 12) & 0xF) << 4;
    pt[n].high_addr = ((addr >> 16) & 0xFFFF);

    /*
     * name    | value | size | desc
     * ---------------------------
     *       P |     1 |    1 | If the entry is present or not
     *     R/W |    rw |    1 | Read/Write:
     *                              0 = Read-only
     *                              1 = Write and Read
     *     U/S |    pl |    1 | User/Supervisor:
     *                              0 = PL3 can't access
     *                              1 = PL3 can access
     *     PWT |     1 |    1 | Page-level write-through:
     *                              0 = writes are cached
     *                              1 = writes are not cached
     *     PCD |     0 |    1 | Page-level cache disable
     *       A |     0 |    1 | Is set if the entry has been accessed
     * Ignored |     0 |    1 | Ignored
     *     PAT |     0 |    1 | 1 = PAT is support, 0 = PAT is not supported
     *       G |     0 |    1 | 1 = The PTE is global, 0 = The PTE is local
     * Ignored |     0 |    3 | Ignored
     *
     * NOTE: G and Ignore are part of pt[n].middle, not pt[n].config!
     */
    pt[n].config =
        (0x01 << 3) | ((0x01 & pl) << 2) | ((0x01 & rw) << 1) | 0x01;
}

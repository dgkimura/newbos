#include <stdint.h>
#include <string.h>

#include <newbos/paging.h>
#include <newbos/printk.h>

#include "memory.h"

#define MAX_NUM_MEMORY_MAP  100
#define FOUR_KB     0x1000
#define PAGING_PL0        0
#define PAGING_READ_WRITE 1

struct memory_map {
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

static uint32_t
align_up(uint32_t n, uint32_t a)
{
    uint32_t m = n % a;
    if (m == 0)
    {
        return n;
    }
    return n + (a - m);
}

static uint32_t
align_down(uint32_t n, uint32_t a)
{
    return n - (n % a);
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
    printk("bitmap vaddr: %X, bitmap paddr: %X, page_frames.len: %u, "
           "bitmap_size: %u, bitmap_pfs: %u\n",
           vaddr, paddr, page_frames.len, bitmap_size, bitmap_pfs);

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
    for (i = 0; i < page_frames.len % 8; ++i) {
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
    struct multiboot_info *minfo)
{
    uint32_t i, addr, len;

    printk("Kernel Address:\n");
    printk(" Physical: [%X ... %X]\n",
           kernel_physical_start, kernel_physical_end);
    printk(" Virtual:  [%X ... %X]\n",
           kernel_virtual_start, kernel_virtual_end);

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
        printk("mmap[%u] -> addr: %X, len: %u, pfs: %u\n",
               i, addr, len, len / FOUR_KB);
    }
}

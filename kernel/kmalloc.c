#include <stddef.h>

#include <newbos/kmalloc.h>
#include <newbos/paging.h>
#include <newbos/printk.h>

/*
 * in units
 */
#define MIN_BLOCK_SIZE  1024
#define FOUR_KB     0x1000

/*
 * malloc() and free() as implemented by K&R
 */

typedef double align;

struct header {
    struct header *next;
    /*
     * Size in "number of header units"
     */
    size_t size;
};
typedef struct header header_t;

/*
 * Empty list to get started
 */
static header_t base;

/*
 * Start of free list
 */
static header_t *freep = 0;

static void *
acquire_more_heap(
    size_t nunits
);

static uint32_t
div_ceil(
    uint32_t num,
    uint32_t den)
{
    return (num - 1) / den + 1;
}

void *
kmalloc(size_t nbytes)
{
    header_t *p, *prevp;
    size_t nunits;


    if (nbytes == 0)
    {
        return NULL;
    }

    if (freep == 0)
    {
        /*
         * No free list yet.
         */
        base.next = freep = &base;
        base.size = 0;
    }

    nunits = (nbytes+sizeof(header_t)-1)/sizeof(header_t) + 1;
    prevp = freep;

    for (p = prevp->next; ; prevp = p, p = p->next)
    {
        if (p->size >= nunits)
        {
            /*
             * The block is big enough.
             */
            if (p->size == nunits)
            {
                /*
                 * Exactly
                 */
                prevp->next = p->next;
            }
            else
            {
                /*
                 * Allocate at the tail end of the block and create a new
                 * header in front of allocated block.
                 */
                p->size -= nunits;

                /*
                 * Make p point to new header.
                 */
                p += p->size;
                p->size = nunits;
            }
            freep = prevp;
            return (void *)(p+1);
        }
        if (p == freep)
        {
            /*
             * Wrapped around free list.
             */
            if ((p = acquire_more_heap(nunits)) == NULL)
            {
                printk("Cannot acquire more memory. memory: %u",
                       nbytes);
                return NULL;
            }
        }
    }
}

static void *
acquire_more_heap(size_t nunits)
{
    uint32_t vaddr, paddr, bytes, page_frames, mapped_mem;
    header_t *p;

    if (nunits < MIN_BLOCK_SIZE)
    {
        nunits = MIN_BLOCK_SIZE;
    }

    page_frames = div_ceil(nunits * sizeof(header_t), FOUR_KB);
    bytes = page_frames * FOUR_KB;

    paddr = pfa_allocate(page_frames);
    if (paddr == 0)
    {
        printk("Could't allocated page frames for kmalloc. "
               "page_frames: %u, bytes: %u\n",
                  page_frames, bytes);
        return NULL;
    }

    vaddr = pdt_kernel_find_next_vaddr(bytes);
    if (vaddr == 0)
    {
        printk("Could't find a virtual address. "
               "paddr: %X, page_frames: %u, bytes: %u\n",
               paddr, page_frames, bytes);
        return NULL;
    }

    mapped_mem = pdt_map_kernel_memory(paddr, vaddr, bytes,
                                       PAGING_READ_WRITE, PAGING_PL0);
    if (mapped_mem < bytes)
    {
        printk("Could't map virtual memory. "
               "vaddr: %X, paddr: %X, page_frames: %u, bytes: %u\n",
                  vaddr, paddr, page_frames, bytes);
        return NULL;
    }

    p = (header_t *) vaddr;
    p->size = bytes / sizeof(header_t);

    kfree((void *)(p+1));

    return freep;
}

void
kfree(void * ap)
{
    header_t *bp, *p;

    if (ap == 0)
    {
        return;
    }

    /*
     * Point to block header.
     */
    bp = (header_t *)ap - 1;
    for (p = freep; !(bp > p && bp < p->next); p = p->next)
    {
        if (p >= p->next && (bp > p || bp < p->next))
        {
            /*
             * Freed block at start of end of arena.
             */
            break;
        }
    }

    if (bp + bp->size == p->next)
    {
        /*
         * Join to upper nbr.
         */
        bp->size += p->next->size;
        bp->next = p->next->next;
    }
    else
    {
        bp->next = p->next;
    }

    if (p + p->size == bp)
    {
        /*
         * Join to lower nbr.
         */
        p->size += bp->size;
        p->next = bp->next;
    }
    else
    {
        p->next = bp;
    }

    /*
     * TODO: If the block is larger than a page frame, give back to pfa!
     */
    freep = p;
}

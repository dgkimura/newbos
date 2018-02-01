#ifndef _NEWBOS_PAGING_H
#define _NEWBOS_PAGING_H

#define PAGE_SIZE 0x1000

#define PAGE_FLAG_USER        0
#define PAGE_FLAG_KERNEL      1
#define PAGE_FLAG_EXISTS      0
#define PAGE_FLAG_ALLOC       2
#define PAGE_FLAG_READONLY    0
#define PAGE_FLAG_READWRITE   4
#define PAGE_FLAG_NOCLEAR     0
#define PAGE_FLAG_CLEAR       8

struct pagetable_entry
{
    uint32_t present      : 1;
    uint32_t readwrite    : 1;
    uint32_t user         : 1;
    uint32_t writethrough : 1;
    uint32_t nocache      : 1;
    uint32_t accessed     : 1;
    uint32_t dirty        : 1;
    uint32_t pagesize     : 1;
    uint32_t globalpage   : 1;
    uint32_t available    : 3;
    uint32_t address      : 20;
};

struct pagetable {
	struct pagetable_entry entry[PAGE_SIZE/4];
};

void
frames_init(
    void
);

void *
frame_allocate(
    void
);

void
frame_free(
    void *address
);

void
pagetable_init(
    struct pagetable *p
);

void
pagetable_alloc(
    struct pagetable *p,
    uint32_t virtual_address,
    uint32_t length,
    uint32_t flags
);

void
pagetable_map(
    struct pagetable *p,
    uint32_t virtual_address,
    uint32_t physical_address,
    uint32_t flags
);

int
pagetable_get(
    struct pagetable *p,
    uint32_t virtual_address,
    uint32_t *physical_address
);

#endif

#include <stddef.h>

#include <newbos/kmalloc.h>
#include <newbos/tty.h>

enum kchunk_state
{
    KCHUNK_STATE_FREE,
    KCHUNK_STATE_USED
};

struct kmalloc_chunk
{
    enum kchunk_state state;
    uint32_t length;
    struct kmalloc_chunk *next;
    struct kmalloc_chunk *prev;
};

static struct kmalloc_chunk *head = 0;

static void
kchunk_split(struct kmalloc_chunk *c, int32_t length)
{
    struct kmalloc_chunk *n = (struct kmalloc_chunk *)(c + length);

    n->state = KCHUNK_STATE_FREE;
    n->length = c->length - length;
    n->prev = c;
    n->next = c->next;

    if (c->next)
    {
        c->next->prev = n;
    }

    c->next = n;
    c->length = length;
}

void *
kmalloc(
    uint32_t size)
{
    struct kmalloc_chunk *c = head;
    while (c && (c->state != KCHUNK_STATE_FREE || c->length < size))
    {
        c = c->next;
    }

    if (!c)
    {
        monitor_write("kmalloc: Not enough free memory to satisfy request.\n");
        return 0;
    }

    if (size - c->length > sizeof(struct kmalloc_chunk) * 2)
    {
        kchunk_split(c, size);
    }

    c->state = KCHUNK_STATE_USED;
    return c + 1;
}

static void
kchunk_merge(struct kmalloc_chunk *c)
{
    if (!c)
    {
        return;
    }

    if (c->state != KCHUNK_STATE_FREE)
    {
        return;
    }

    if (c->next && c->next->state == KCHUNK_STATE_FREE)
    {
        c->length += c->next->length;
        if (c->next->next)
        {
            c->next->next->prev = c;
        }
        c->next = c->next->next;
    }
}

void
kfree(
    void *p)
{
    struct kmalloc_chunk *c = (struct kmalloc_chunk *)p;
    c -= 1;

    if (c->state != KCHUNK_STATE_USED)
    {
        monitor_write("bad kfree!\n");
        return;
    }

    c->state = KCHUNK_STATE_FREE;
    kchunk_merge(c);
    kchunk_merge(c->prev);
}

void
kmalloc_init(
    void *start,
    uint32_t length)
{
    head = (struct kmalloc_chunk *)start;
    head->state = KCHUNK_STATE_FREE;
    head->length = length;
    head->next = NULL;
    head->prev = NULL;
}

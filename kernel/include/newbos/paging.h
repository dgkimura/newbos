#ifndef _NEWBOS_PAGING_H
#define _NEWBOS_PAGING_H

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

#endif

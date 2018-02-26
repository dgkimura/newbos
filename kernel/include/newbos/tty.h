#ifndef _NEWBOS_TTY_H
#define _NEWBOS_TTY_H

#include <stdint.h>

void
tty_write_byte(
    uint8_t b
);

void
tty_write_string(
    char const *s
);

void
tty_write_hex(
    uint32_t i
);

void
tty_clear(
    void
);

void
tty_move_cursor(
    uint16_t row,
    uint16_t col
);

#endif

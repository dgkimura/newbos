#include <newbos/tty.h>

#include "io.h"

#define KERNEL_START_VADDR  0xC0000000
#define TTY_MEMORY KERNEL_START_VADDR + 0x000B8000

#define TTY_NUM_COLS 80
#define TTY_NUM_ROWS 25

#define TTY_CURSOR_DATA_PORT 0x3D5
#define TTY_CURSOR_INDEX_PORT 0x3D4

#define TTY_HIGH_BYTE 14
#define TTY_LOW_BYTE 15

#define BLACK_ON_WHITE 0x0F

#define TO_ADDRESS(row, col) (fb + 2 * (row * TTY_NUM_COLS + col))

#define TTY_BACKSPACE_ASCII 8

static uint8_t *fb = (uint8_t *) TTY_MEMORY;

static uint16_t cursor_pos;

static uint8_t
read_cell(
    uint32_t row,
    uint32_t col)
{
    uint8_t *cell = TO_ADDRESS(row, col);
    return *cell;
}

static void
write_cell(
    uint8_t *cell,
    uint8_t b)
{
    cell[0] = b;
    cell[1] = BLACK_ON_WHITE;
}

static void
write_at(
    uint8_t b,
    uint32_t row,
    uint32_t col)
{
    uint8_t *cell = TO_ADDRESS(row, col);
    write_cell(cell, b);
}


static void
set_cursor(
    uint16_t loc)
{
    outb(TTY_CURSOR_INDEX_PORT, TTY_HIGH_BYTE);
    outb(TTY_CURSOR_DATA_PORT, loc >> 8);
    outb(TTY_CURSOR_INDEX_PORT, TTY_LOW_BYTE);
    outb(TTY_CURSOR_DATA_PORT, loc);
}

static void
move_cursor_forward()
{
    cursor_pos++;
    set_cursor(cursor_pos);
}

static void
move_cursor_back()
{
    if (cursor_pos != 0)
    {
        cursor_pos--;
        set_cursor(cursor_pos);
    }
}

static void
move_cursor_down()
{
    cursor_pos += TTY_NUM_COLS;
    set_cursor(cursor_pos);
}

static void
move_cursor_start()
{
    cursor_pos -= cursor_pos % TTY_NUM_COLS;
    set_cursor(cursor_pos);
}

static void
scroll()
{
    uint32_t r, c;
    for (r = 1; r < TTY_NUM_ROWS; ++r)
    {
        for (c = 0; c < TTY_NUM_COLS; ++c)
        {
            write_at(read_cell(r, c), r - 1, c);
        }
    }

    for (c = 0; c < TTY_NUM_COLS; ++c)
    {
        write_at(' ', TTY_NUM_ROWS - 1, c);
    }
}

void
tty_write_byte(
    uint8_t b)
{
    if (b != '\n' && b != '\t' && b != TTY_BACKSPACE_ASCII)
    {
        uint8_t *cell = fb + 2 * cursor_pos;
        write_cell(cell, b);
    }

    if (b == '\n')
    {
        move_cursor_down();
        move_cursor_start();
    }
    else if (b == TTY_BACKSPACE_ASCII)
    {
        move_cursor_back();
        uint8_t *cell = fb + 2 * cursor_pos;
        write_cell(cell, ' ');
    }
    else if (b == '\t')
    {
        int i;
        for (i = 0; i < 4; ++i)
        {
            tty_write_byte(' ');
        }
    }
    else
    {
        move_cursor_forward();
    }

    if (cursor_pos >= TTY_NUM_COLS * TTY_NUM_ROWS)
    {
        scroll();
        tty_move_cursor(24, 0);
    }
}

void
tty_write_string(
    char const *s)
{
    while (*s != '\0')
    {
        tty_write_byte(*s++);
    }
}

void
tty_write_hex(
    uint32_t n)
{
    char *chars = "0123456789ABCDEF";
    unsigned char b = 0;
    int i;

    tty_write_string("0x");

    for (i = 7; i >= 0; --i)
    {
        b = (n >> i*4) & 0x0F;
        tty_write_byte(chars[b]);
    }
}

void
tty_write_int(
    uint32_t n)
{
    char *chars = "0123456789";
    unsigned char reversed[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int i = 0;

    if (n == 0)
    {
        tty_write_byte('0');
        return;
    }

    while (n > 0)
    {
        reversed[++i] = n % 10;
        n /= 10;
    }
    for (; 0<i; i--)
    {
        tty_write_byte(chars[reversed[i]]);
    }
}

void
tty_clear()
{
    uint8_t i, j;
    for (i = 0; i < TTY_NUM_ROWS; ++i)
    {
        for (j = 0; j < TTY_NUM_COLS; ++j)
        {
            write_at(' ', i, j);
        }
    }
    tty_move_cursor(0, 0);
}

void
tty_move_cursor(
    uint16_t row,
    uint16_t col)
{
    uint16_t loc = row*TTY_NUM_COLS + col;
    cursor_pos = loc;
    set_cursor(loc);
}

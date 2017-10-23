#ifndef _NEWBOS_TTY_H
#define _NEWBOS_TTY_H

#include <stdint.h>

uint8_t inb(uint16_t port);

void outb(uint16_t port, uint8_t value);

// Write a single character out to the screen.
void monitor_put(char c);

// Clear the screen to all black.
void monitor_clear();

// Output a null-terminated ASCII string to the monitor.
void monitor_write(char *c);

void monitor_write_dec(uint32_t n);

void monitor_write_hex(uint32_t n);

#endif

#ifndef _NEWBOS_TTY_H
#define _NEWBOS_TTY_H

#include <stdint.h>

// Write a single character out to the screen.
void monitor_put(char c);

// Clear the screen to all black.
void monitor_clear();

// Output a null-terminated ASCII string to the monitor.
void monitor_write(char *c);

void monitor_write_dec(uint32_t n);

#endif // MONITOR_H


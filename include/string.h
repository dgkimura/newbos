#ifndef _NEWBOS_STRING_H_

#include <stddef.h>

int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *__restrict dst, const void *__restrict src, size_t n);
void *memmove(void *dst, const void *src, size_t len);
void *memset(void *b, int c, size_t len);
size_t strlen(const char *s);

#endif

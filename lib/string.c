#include <string.h>

int
memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *_s1 = (const unsigned char*)s1;
    const unsigned char *_s2 = (const unsigned char*)s2;
    for (size_t i = 0; i < n; i++)
    {
        if (_s1[i] < _s2[i])
        {
            return -1;
        }
        else if (_s1[i] > _s2[i])
        {
            return 1;
        }
    }
    return 0;
}

void *
memcpy(void *__restrict dst, const void *__restrict src, size_t n)
{
    unsigned char *_dst= (unsigned char*)dst;
    const unsigned char *_src = (const unsigned char*)src;
    for (size_t i = 0; i < n; i++)
    {
        _dst[i] = _src[i];
    }
    return dst;
}

void *
memmove(void *dst, const void *src, size_t len)
{
    unsigned char *_dst= (unsigned char*)dst;
    const unsigned char *_src = (const unsigned char*)src;
    if (dst < src)
    {
        for (size_t i = 0; i < len; i++)
        {
            _dst[i] = _src[i];
        }
    }
    else
    {
        for (size_t i = len; i != 0; i--)
        {
            _dst[i - 1] = _src[i - 1];
        }
    }
    return dst;
}

void *
memset(void *b, int c, size_t len)
{
    unsigned char *_b = (unsigned char *)b;
    for (size_t i = 0; i < len; i++)
    {
        _b[i] = (unsigned char)c;
    }
    return _b;
}

size_t
strlen(const char *s)
{
    size_t len = 0;
    while (s[len++]);

    return len;
}

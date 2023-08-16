#include <fakedyld/fakedyld.h>

void *memcpy(void *dst, const void *src, size_t n)
{
    const char *s = src;
    char       *d = dst;
    int        pos = 0, dir = 1;
    
    if (d > s) {
        dir = -1;
        pos = n - 1;
    }
    
    while (n--) {
        d[pos] = s[pos];
        pos += dir;
    }
    
    return dst;
}

void *memmove(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

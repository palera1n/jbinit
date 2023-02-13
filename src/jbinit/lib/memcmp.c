#include <jbinit.h>

int
memcmp(const void *b1, const void *b2, register size_t n)
{
    unsigned char *m1 = (unsigned char *)b1;
    unsigned char *m2 = (unsigned char *)b2;
    
    while (n-- && (*m1 == *m2)) {
        m1++;
        m2++;
    }
    
    return ((n < 0) ? 0 : (*m1 - *m2));
}

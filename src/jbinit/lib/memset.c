#include "jbinit.h"

void memset(void *dst, int c, size_t n)
{
  uint8_t *d = (uint8_t *)dst;
  for (size_t i = 0; i < n; i++)
    *d++ = c;
}

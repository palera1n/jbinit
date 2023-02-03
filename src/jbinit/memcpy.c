#include "jbinit.h"

void memcpy(void *dst, void *src, size_t n)
{
  uint8_t *s = (uint8_t *)src;
  uint8_t *d = (uint8_t *)dst;
  for (size_t i = 0; i < n; i++)
    *d++ = *s++;
}

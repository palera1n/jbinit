#include "jbinit.h"

char *strcat(char *dest, char *src)
{
  char *dest_end = dest + strlen(dest);
  for (uint64_t i = 0; src[i] != '\0'; i++)
  {
    *dest_end = src[i];
    dest_end++;
  }
  dest_end = NULL;
  return dest;
}

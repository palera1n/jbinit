/* This file is part of Embedded Artistry libc */

#include <fakedyld/fakedyld.h>

char *strchr(const char *p, int ch) {
    char c;
    
    c = ch;
    for (;; ++p) {
        if (*p == c)
            return ((char *)p);
        if (*p == '\0')
            return (NULL);
    }
}

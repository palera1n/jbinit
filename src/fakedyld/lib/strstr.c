#include <fakedyld/fakedyld.h>

char *strstr(const char *string, char *substring) {
    register char *a, *b;
    
    /* First scan quickly through the two strings looking for a
     * single-character match.  When it's found, then compare the
     * rest of the substring.
     */
    
    b = substring;
    if (*b == 0) {
        return (char *)string;
    }
    for ( ; *string != 0; string += 1) {
        if (*string != *b) {
            continue;
        }
        a = (char *)string;
        while (1) {
            if (*b == 0) {
                return (char *)string;
            }
            if (*a++ != *b++) {
                break;
            }
        }
        b = substring;
    }
    return NULL;
}

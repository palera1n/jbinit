#include <stddef.h>

void bzero (void *s, size_t n) {
	for (size_t i = 0; i < n; i++) {
		*((char*)s + i) = '\0';
	}
}

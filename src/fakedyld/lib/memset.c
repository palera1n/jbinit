/* This file is part of Embedded Artistry libc */

#include <stdint.h>
#include <stddef.h>

void* memset(void* dest, int c, size_t n)
{
	unsigned char* s = dest;
	size_t k;

	/* Fill head and tail with minimal branching. Each
	 * conditional ensures that all the subsequently used
	 * offsets are well-defined and in the dest region. */

	if(!n)
	{
		return dest;
	}

	s[0] = s[n - 1] = (unsigned char)c;

	if(n <= 2)
	{
		return dest;
	}

	s[1] = s[n - 2] = (unsigned char)c;
	s[2] = s[n - 3] = (unsigned char)c;

	if(n <= 6)
	{
		return dest;
	}

	s[3] = s[n - 4] = (unsigned char)c;

	if(n <= 8)
	{
		return dest;
	}

	/* Advance pointer to align it at a 4-byte boundary,
	 * and truncate n to a multiple of 4. The previous code
	 * already took care of any head/tail that get cut off
	 * by the alignment. */

	k = -(uintptr_t)s & 3;
	s += k;
	n -= k;
	n &= (unsigned long)-4;
	n /= 4;

	// Cast to void first to prevent alignment warning
	uint32_t* ws = (uint32_t*)(void*)s;
	uint32_t wc = c & 0xFF;
	wc |= ((wc << 8) | (wc << 16) | (wc << 24));

	/* Pure C fallback with no aliasing violations. */
	for(; n; n--, ws++)
	{
		*ws = wc;
	}

	return dest;
}

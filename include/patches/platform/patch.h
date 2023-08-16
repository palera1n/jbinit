#ifndef _PLATFORM_PATCH_H
#define _PLATFORM_PATCH_H
#include <stdint.h>
#include <stddef.h>

void patch_platform_check(void *real_buf, void *dyld_buf, size_t dyld_len, uint32_t platform);

#endif

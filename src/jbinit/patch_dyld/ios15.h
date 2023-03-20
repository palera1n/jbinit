#ifndef _PLATFORM_IOS15_H
#define _PLATFORM_IOS15_H
#include <stdint.h>
#include <jbinit.h>

void patch_platform_check15(void *dyld_buf, size_t dyld_len, uint32_t platform);

#endif

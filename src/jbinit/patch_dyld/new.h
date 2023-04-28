#ifndef _PLATFORM_NEW_H
#define _PLATFORM_NEW_H
#include <stdint.h>
#include <jbinit.h>

void patch_platform_check_new(void *real_buf, void *dyld_buf, size_t dyld_len, uint32_t platform);

#endif
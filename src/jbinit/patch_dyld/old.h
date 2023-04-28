#ifndef _PLATFORM_OLD_H
#define _PLATFORM_OLD_H
#include <stdint.h>
#include <jbinit.h>

void patch_platform_check_old(void *real_buf, void *dyld_buf, size_t dyld_len, uint32_t platform);

#endif

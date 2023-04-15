#ifndef _PLATFORM_IOS16_SHC_H
#define _PLATFORM_IOS16_SHC_H
#include <stdint.h>
#include <jbinit.h>

uint32_t *get_shc_region(void *buf);
void copy_shc(int platform, char target_reg);

#endif
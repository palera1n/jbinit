#ifndef _PLATFORM_SHC_H
#define _PLATFORM_SHC_H
#include <stdint.h>

uint32_t *get_shc_region(void *buf);
uint32_t *copy_shc(int platform, uint32_t jmp);

#endif
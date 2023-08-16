#ifndef _MULTI_H
#define _MULTI_H
#include <stdint.h>

void *pf_va_to_ptr(void *buf, uint64_t addr);
uint64_t pf_ptr_to_va(void *buf, void *ptr);

#endif
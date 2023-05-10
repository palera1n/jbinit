#ifndef _PLOOSHFINDER_H
#define _PLOOSHFINDER_H
#include <stdint.h>
#include <jbinit.h>
#include <stdbool.h>

// util macros
#define nop 0xd503201f
#define ret 0xd65f03c0

// utils for finding
uint32_t *pf_find_next(uint32_t *stream, uint32_t count, uint32_t match, uint32_t mask);
uint32_t *pf_find_prev(uint32_t *stream, uint32_t count, uint32_t match, uint32_t mask);
int64_t pf_adrp_offset(uint32_t adrp);
uint32_t *pf_follow_veneer(void *buf, uint32_t *stream);
uint32_t *pf_follow_branch(void *buf, uint32_t *stream);
void *pf_follow_xref(void *buf, uint32_t *stream);

#endif

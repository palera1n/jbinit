#ifndef _PLOOSHFINDER32_H
#define _PLOOSHFINDER32_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct pf_patch32_t {
    uint32_t *matches;
    uint32_t *masks;
    bool disabled;
    uint32_t count;
    bool (*callback)(struct pf_patch32_t *patch, void *stream);
};

struct pf_patchset32_t {
    struct pf_patch32_t *patches;
    uint32_t count;
    void (*handler)(void *buf, size_t size, struct pf_patchset32_t patch);
};

// patch utils
bool pf_maskmatch32(uint32_t insn, uint32_t match, uint32_t mask);
void pf_find_maskmatch32(void *buf, size_t size, struct pf_patchset32_t patchset);

// utils for finding
int32_t pf_signextend_32(int32_t val, uint8_t bits);

#endif

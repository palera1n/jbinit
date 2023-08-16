#ifndef _PLOOSHFINDER64_H
#define _PLOOSHFINDER64_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct pf_patch64_t {
    uint64_t *matches;
    uint64_t *masks;
    bool disabled;
    uint32_t count;
    bool (*callback)(struct pf_patch64_t *patch, void *stream);
};

struct pf_patchset64_t {
    struct pf_patch64_t *patches;
    uint32_t count;
    void (*handler)(void *buf, size_t size, struct pf_patchset64_t patch);
};

// patch utils
bool pf_maskmatch64(uint64_t insn, uint64_t match, uint64_t mask);
void pf_find_maskmatch64(void *buf, size_t size, struct pf_patchset64_t patchset);

// utils for finding
int64_t pf_signextend_64(int64_t val, uint8_t bits);

#endif

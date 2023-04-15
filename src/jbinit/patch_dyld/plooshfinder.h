#ifndef _PLOOSHFINDER_H
#define _PLOOSHFINDER_H

#include <stdint.h>
#include <jbinit.h>
#include <stdbool.h>

#include "utils.h"
#include "macho.h"
#include "macho_defs.h"

// util macros
#define nop 0xd503201f
#define ret 0xd65f03c0

struct pf_patch_t {
    void *matches;
    void *masks;
    uint32_t count;
    bool disabled;
    bool (*callback)(struct pf_patch_t patch, void *stream);
};
struct pf_patch32_t {
    uint32_t *matches;
    uint32_t *masks;
    uint32_t count;
    bool disabled;
    bool (*callback)(struct pf_patch32_t patch, void *stream);
};

struct pf_patch64_t {
    uint64_t *matches;
    uint64_t *masks;
    uint32_t count;
    bool disabled;
    bool (*callback)(struct pf_patch64_t patch, void *stream);
};

struct pf_patchset_t {
    struct pf_patch_t *patches;
    uint32_t count;
    void (*handler)(void *buf, size_t size, struct pf_patchset_t patch);
};

struct pf_patchset32_t {
    struct pf_patch32_t *patches;
    uint32_t count;
    void (*handler)(void *buf, size_t size, struct pf_patchset32_t patch);
};
struct pf_patchset64_t {
    struct pf_patch64_t *patches;
    uint32_t count;
    void (*handler)(void *buf, size_t size, struct pf_patchset64_t patch);
};


// patch utils
struct pf_patch32_t pf_construct_patch32(uint32_t matches[], uint32_t masks[], uint32_t count, bool (*callback)(struct pf_patch32_t patch, void *stream));
struct pf_patch64_t pf_construct_patch64(uint64_t matches[], uint64_t masks[], uint32_t count, bool (*callback)(struct pf_patch64_t patch, void *stream));
struct pf_patchset32_t pf_construct_patchset32(struct pf_patch32_t *patches, uint32_t count, void (*handler)(void *buf, size_t size, struct pf_patchset32_t patchset));
struct pf_patchset64_t pf_construct_patchset64(struct pf_patch64_t *patches, uint32_t count, void (*handler)(void *buf, size_t size, struct pf_patchset64_t patchset));
void pf_patchset_emit32(void *buf, size_t size, struct pf_patchset32_t patchset);
void pf_patchset_emit64(void *buf, size_t size, struct pf_patchset64_t patchset);
void pf_disable_patch32(struct pf_patch32_t patch);
void pf_disable_patch64(struct pf_patch64_t patch);

// utils for finding
void pf_find_maskmatch32(void *buf, size_t size, struct pf_patchset32_t patchset);
void pf_find_maskmatch64(void *buf, size_t size, struct pf_patchset64_t patchset);
uint32_t *pf_find_next(uint32_t *stream, uint32_t count, uint32_t match, uint32_t mask);
uint32_t *pf_find_prev(uint32_t *stream, uint32_t count, uint32_t match, uint32_t mask);
int32_t pf_signextend_32(int32_t val, uint8_t bits);
int64_t pf_signextend_64(int64_t val, uint8_t bits);
uint32_t *pf_follow_branch(uint32_t *insn);
int64_t pf_adrp_offset(uint32_t adrp);
void *pf_follow_xref(uint32_t *stream);

#endif
